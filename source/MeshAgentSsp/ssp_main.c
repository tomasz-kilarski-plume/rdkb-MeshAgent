/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
* @file ssp_main.c
* 
* @description This file is used to manage the dbus call and stack trace.
*
*/
#ifdef __GNUC__
#ifndef _BUILD_ANDROID
#include <execinfo.h>
#endif
#endif

#include "ssp_global.h"
#ifdef INCLUDE_BREAKPAD
#include "breakpad_wrapper.h"
#endif
#include "stdlib.h"
#include "ccsp_dm_api.h"
#include "meshagent.h"
#include "safec_lib_common.h"
#include "webconfig_framework.h"
#include "print_uptime.h"
#include <sys/sysinfo.h>
#include "secure_wrapper.h"
#include <systemd/sd-daemon.h>

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/*None*/

/*----------------------------------------------------------------------------*/
/*                               File scoped variables                              */
/*----------------------------------------------------------------------------*/
char  g_Subsystem[32] = {0};

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

PCCSP_COMPONENT_CFG gpMeshAgentCfg = NULL;
/**
 * @brief This functionality helps in approaching the bus deamon to create and engage the components.
 */
int  cmd_dispatch(int  command)
{
	
    switch ( command )
    {
        case    'e' :

            MeshInfo("Connect to bus daemon...\n");

            {
                char                            CName[256];

                if ( g_Subsystem[0] != 0 )
                {
                    _ansc_sprintf(CName, "%s%s", g_Subsystem, gpMeshAgentCfg->ComponentId);
                }
                else
                {
                    _ansc_sprintf(CName, "%s", gpMeshAgentCfg->ComponentId);
                }

                MeshInfo("ssp_Mbi_MessageBusEngage() called\n");
				ssp_Mbi_MessageBusEngage(CName,
                        				 CCSP_MSG_BUS_CFG,
				                         gpMeshAgentCfg->DbusPath);
            }
            ssp_create(gpMeshAgentCfg);
            ssp_engage(gpMeshAgentCfg);
            break;

        case    'm':
                AnscPrintComponentMemoryTable(pComponentName);
                break;

        case    't':
                AnscTraceMemoryTable();
                break;

        case    'c':
                ssp_cancel(gpMeshAgentCfg);
                break;

        default:
            break;
    }

    return 0;
}

/**
 * @brief Bus platform initialization to engage the component to CR(Component Registrar).
 */
int msgBusInit(const char *pComponentName)
{
    BOOL                            bRunAsDaemon       = TRUE;
    int                             cmdChar            = 0;
	
    extern ANSC_HANDLE bus_handle;
    char *subSys            = NULL;  
    DmErr_t    err;
    errno_t rc = -1;

	gpMeshAgentCfg = (PCCSP_COMPONENT_CFG)AnscAllocateMemory(sizeof(CCSP_COMPONENT_CFG));
    if (gpMeshAgentCfg)
    {   
        CcspComponentLoadCfg(CCSP_MESHAGENT_START_CFG_FILE, gpMeshAgentCfg);
    }   
    else
    {   
        printf("Insufficient resources for start configuration, quit!\n");
        exit(1);
    }   
    
    /* Set the global pComponentName */
    pComponentName = gpMeshAgentCfg->ComponentName;
    MeshInfo("[MeshAgent] msgBusInit called with %s\n", pComponentName);

    rc = strcpy_s(g_Subsystem,sizeof(g_Subsystem),"eRT.");
    if(rc != EOK)
    {
	ERR_CHK(rc);
	return 1;
    }

#ifdef INCLUDE_BREAKPAD
    breakpad_ExceptionHandler();
#endif

    cmd_dispatch('e');

    subSys = NULL;      /* use default sub-system */

    err = Cdm_Init(bus_handle, subSys, NULL, NULL, pComponentName);
    if (err != CCSP_SUCCESS)
    {
		MeshError("Cdm_Init Failed!!!!\n");
        fprintf(stderr, "Cdm_Init: %s\n", Cdm_StrError(err));
        exit(1);
    }

    check_component_crash("/tmp/meshagent_initialized");
    v_secure_system("touch /tmp/meshagent_initialized");
    MeshInfo("msgBusInit - /tmp/meshagent_initialized created\n");
    pthread_t tid;
    pthread_create(&tid, NULL, Cosa_print_uptime_meshagent, NULL);

    if ( bRunAsDaemon )
    {
        sd_notify(0, "READY=1");
        return 1; //RunAsDaemon
    }
    else
    {
        while ( cmdChar != 'q' )
        {
            cmdChar = getchar();

            cmd_dispatch(cmdChar);
        }
    }

    err = Cdm_Term();
    if (err != CCSP_SUCCESS)
    {
		MeshError("msgBusInit - Cdm_Term() failed!!! exit(1)\n");
    	fprintf(stderr, "Cdm_Term: %s\n", Cdm_StrError(err));
    	exit(1);
    }
    ssp_cancel(gpMeshAgentCfg);
    return 0; //Success
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

int CheckAndGetDevicePropertiesEntry( char *pOutput, int size, char *sDevicePropContent )
{
    FILE    *fp1         = NULL;
    char     buf[ 1024 ] = { 0 },
            *urlPtr      = NULL;
    int      ret         = -1;
    errno_t rc = -1;

    // Read the device.properties file
    fp1 = fopen( "/etc/device.properties", "r" );

    if ( NULL == fp1 )
    {
        CcspTraceError(("Error opening properties file! \n"));
        return -1;
    }

    while ( fgets( buf, sizeof( buf ), fp1 ) != NULL )
    {
        // Look for Device Properties Passed Content
        if ( strstr( buf, sDevicePropContent ) != NULL )
        {
            buf[strcspn( buf, "\r\n" )] = 0; // Strip off any carriage returns

            // grab content from string(entry)
            urlPtr = strstr( buf, "=" );
            urlPtr++;

            rc = strncpy_s(pOutput, size, urlPtr,size);
            if(rc != EOK)
            {
                ERR_CHK(rc);
                return -1;
            }
	    
	    ret=0;
            break;
        }
    }

    fclose( fp1 );
    return ret;
}

void* Cosa_print_uptime_meshagent( void* arg )
{
    UNREFERENCED_PARAMETER(arg);
    pthread_detach(pthread_self());
    struct sysinfo l_sSysInfo;
    sysinfo(&l_sSysInfo);
    char uptime[16] = {0};
    snprintf(uptime, sizeof(uptime), "%ld", l_sSysInfo.uptime);
    CcspTraceWarning(("print_uptime: Mesh uptime is %s\n",uptime));
    print_uptime("boot_to_meshagent_uptime",NULL, uptime);
    return NULL;
}
