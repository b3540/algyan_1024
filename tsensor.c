/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include <string.h>

static void HTS221_Get_Temperature(I2C_HandleTypeDef *hi2c, int16_t *value);

/*
	Sensor data read
*/
void TestHTS221( I2C_HandleTypeDef *hi2c, int16_t *Temperature )
{
	HAL_StatusTypeDef	status;
	static uint8_t				readdata[8];

	/* buffer clear */
	memset( readdata, 0x00, sizeof(uint8_t)*8 );

	/* WHO_AM_I:RegAddr 0x0F, size 1 */
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x0F, 1, &readdata[0], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}
	
	if( readdata[0] != 0xBC )
	{
		/* ERROR */
		printf( "HTS221 WHO_AM_I read fair\n" );
		return;
	}

	/* CTRLREG1:RegAddr 0x20, size 1 */
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x20, 1, &readdata[0], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}

	/* CTRLREG1:RegAddr 0x20, size 1 */
	readdata[0] = 0x81;
	status = HAL_I2C_Mem_Write( hi2c, 0xBF, 0x20, 1, &readdata[0], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Write faild.(%0x08x)\n", status );
		return;
	}

	/*
		temperature read
	*/
	HTS221_Get_Temperature( hi2c, Temperature );

	return;
}

static void HTS221_Get_Temperature(I2C_HandleTypeDef *hi2c, int16_t *value)
{
	HAL_StatusTypeDef	status;
	int16_t T0_out, T1_out, T_out, T0_degC_x8_u16, T1_degC_x8_u16;
	int16_t T0_degC, T1_degC;
	uint8_t buffer[4], tmp;
	uint32_t tmp32;

	/*1. Read from 0x32 & 0x33 registers the value of coefficients T0_degC_x8 and T1_degC_x8*/
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x32, 1, &buffer[0], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x33, 1, &buffer[1], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}

	/*2. Read from 0x35 register the value of the MSB bits of T1_degC and    T0_degC */
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x35, 1, &tmp, 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}

	/*Calculate the T0_deg C and T1_deg C values*/
	T0_degC_x8_u16 = (((uint16_t)(tmp & 0x03)) << 8) | ((uint16_t)buffer[0]);
	T1_degC_x8_u16 = (((uint16_t)(tmp & 0x0C)) << 6) | ((uint16_t)buffer[1]);
	T0_degC = T0_degC_x8_u16>>3;
	T1_degC = T1_degC_x8_u16>>3;

	/*3. Read from 0x3C & 0x3D registers the value of T0_OUT*/
	/*4. Read from 0x3E & 0x3F registers the value of T1_OUT*/
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x3C, 1, &buffer[0], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x3D, 1, &buffer[1], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x3E, 1, &buffer[2], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x3F, 1, &buffer[3], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}

	T0_out = (((uint16_t)buffer[1])<<8) | (uint16_t)buffer[0];
	T1_out = (((uint16_t)buffer[3])<<8) | (uint16_t)buffer[2];

	/* 5.Read from 0x2A & 0x2B registers the value T_OUT (ADC _OUT).*/
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x2a, 1, &buffer[0], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}
	status = HAL_I2C_Mem_Read( hi2c, 0xBF, 0x2b, 1, &buffer[1], 1, 0x1000 );
	if( status != HAL_OK )
	{
		/* ERROR */
		printf( "HAL_I2C_Mem_Read faild.(%0x08x)\n", status );
		return;
	}
	T_out = (((uint16_t)buffer[1])<<8) | (uint16_t)buffer[0];

	/* 6. Compute the Temperature value by linea r interpolation*/
	tmp32 = ((uint32_t)(T_out - T0_out)) * ((uint32_t)(T1_degC - T0_degC)*10);
	*value = tmp32 /(T1_out - T0_out) + T0_degC*10;

	return;
}
