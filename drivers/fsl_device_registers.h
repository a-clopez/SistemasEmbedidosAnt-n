#ifndef __FSL_DEVICE_REGISTERS_H__
#define __FSL_DEVICE_REGISTERS_H__

/*
 * Include the cpu specific register header files.
 *
 * The CPU macro should be declared in the project or makefile.
 */
#if (defined(CPU_MKL46Z128VLH4) || defined(CPU_MKL46Z128VLL4) || defined(CPU_MKL46Z128VMC4) || \
    defined(CPU_MKL46Z256VLH4) || defined(CPU_MKL46Z256VLL4) || defined(CPU_MKL46Z256VMC4) || \
    defined(CPU_MKL46Z256VMP4))

#define KL46Z4_SERIES

/* CMSIS-style register definitions */
#include "MKL46Z4.h"
/* CPU specific feature definitions */
#include "MKL46Z4_features.h"

#else
    #error "No valid CPU defined!"
#endif

#endif /* __FSL_DEVICE_REGISTERS_H__ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
