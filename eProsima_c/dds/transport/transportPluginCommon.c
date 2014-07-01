#include "transportPluginCommon.h"
#include "../../sys/eProsimaDL.h"
#include "../../macros/snprintf.h"

#include <dds_c/dds_c_string.h>
#include <dds_c/dds_c_infrastructure.h>
#include <osapi/osapi_heap.h>

#include <stdio.h>
#include <stdarg.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_DATA_LENGTH 16383
#define MAX_PACKET_SIZE 65536

// Forward Declarations
unsigned long getDeviceGuid(char* buff, unsigned long bufferLength);

/****************
*
*  LogFile for the library.
*
*/
static FILE *logFile = NULL;

static struct RTIOsapiSemaphore *log_mutex = NULL;

void log_debug(const char *text)
{
	if(RTIOsapiSemaphore_take(log_mutex, NULL) == RTI_OSAPI_SEMAPHORE_STATUS_OK) {
		fprintf(logFile, "Thread_%d: %s\n",RTIOsapiThread_getCurrentThreadID(), text );
		if(RTIOsapiSemaphore_give(log_mutex)!= RTI_OSAPI_SEMAPHORE_STATUS_OK) {
		}
	}
	else
	{
		fprintf(stderr, "Thread_%d - %s: failed to take mutex\n", RTIOsapiThread_getCurrentThreadID(), __FUNCTION__);
	}
	fflush(logFile);
}

void log_debugf(const char *format, ...)
{
	va_list arg_ptr ;
	if(RTIOsapiSemaphore_take(log_mutex, NULL) == RTI_OSAPI_SEMAPHORE_STATUS_OK) {
		va_start( arg_ptr, format ) ;
		vfprintf(logFile,format,arg_ptr) ;
		va_end(arg_ptr);
		if(RTIOsapiSemaphore_give(log_mutex)!= RTI_OSAPI_SEMAPHORE_STATUS_OK) {
		}
	}
	else
	{
		fprintf(stderr, "Thread_%d - %s: failed to take mutex\n", RTIOsapiThread_getCurrentThreadID(), __FUNCTION__);
	}
	fflush(logFile);
}


void log_hexdump(const char *text, const char *buf, int len, int bytesPerLine)
{
	size_t i;
	int count = 1;
	if(text != NULL)
	{
		log_debug(text);
	}
	for(i=0; i < len; i++)
	{
		fprintf(logFile," %02X", ((const unsigned char*)buf)[i]);
		if(count%bytesPerLine == 0)
		{    
			fputc('\n', logFile);
		}
		count++;
	}
	fflush(logFile);
}

void log_address(const NDDS_Transport_Address_t *address)
{
	char buf[64];
	NDDS_Transport_Address_to_string(address, buf, 64);
	log_debug(buf);
	fflush(logFile);
}

void log_rtps_message(const char *text, const NDDS_Transport_Buffer_t buffer_in[], RTI_INT32 buffer_count_in, int bytesPerLine)
{
	int i = 0;
	if(RTIOsapiSemaphore_take(log_mutex, NULL) == RTI_OSAPI_SEMAPHORE_STATUS_OK) {
		log_debugf("Thread_%d:\n", RTIOsapiThread_getCurrentThreadID());
		for(i = 0; i < buffer_count_in; i++)
		{
			log_hexdump(i == 0 ? text : NULL, buffer_in[i].pointer, buffer_in[i].length, bytesPerLine);
		}
		fputc('\n', logFile);
		if(RTIOsapiSemaphore_give(log_mutex)!= RTI_OSAPI_SEMAPHORE_STATUS_OK) {
		}
	}
	else
	{
		fprintf(stderr, "Thread_%d - %s: failed to take mutex\n", RTIOsapiThread_getCurrentThreadID(), __FUNCTION__);
	}
	fflush(logFile);
}

void log_init(const char *fileName)
{
    const char* const METHOD_NAME = "log_init";

	if(log_mutex == NULL)
	{
		log_mutex = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_MUTEX, NULL);

		if(log_mutex == NULL)
        {
			fprintf(stdout, "ERROR<%s>: Can't create log mutex\n", METHOD_NAME);
		}
	}

	if(logFile == NULL)
	{
        if(fileName != NULL)
        {
            if(RTIOsapiSemaphore_take(log_mutex, NULL) == RTI_OSAPI_SEMAPHORE_STATUS_OK)
            {
                logFile = fopen(fileName, "a");

                if(RTIOsapiSemaphore_give(log_mutex)!= RTI_OSAPI_SEMAPHORE_STATUS_OK)
                {
                    fprintf(stdout, "Thread_%d - %s: failed to give log mutex\n", RTIOsapiThread_getCurrentThreadID(), __FUNCTION__);
                }
            }
        }
        if(logFile == NULL)
		{
			logFile = stdout;
		}
	}
}

/**
* \brief This function gets the properties of the subtransport.
*
* \param transportProperties Structure with the transport properties. Cannot be NULL.
* \param subtransportProperties Structure where the subtransport properties will be stored. Cannot be NULL.
* The structure must be initialized.
* \param subtransportName The name of the subtransport.
*/
void getSubtransportProperties(const struct DDS_PropertyQosPolicy *transportProperties, struct DDS_PropertyQosPolicy *subtransportProperties,
							   const char *subtransportName);

/* Implementation */

NDDS_Transport_create_plugin loadLibrary(const char *libraryName,
										 const char *functionName)
{
	const char* const METHOD_NAME = "loadLibrary";
	NDDS_Transport_create_plugin functionPointer = NULL;

	if(libraryName != NULL && functionName != NULL)
	{
		void *libraryHandle;

		libraryHandle = eProsimaLoadLibrary(libraryName);

		if(libraryHandle != 0)
		{
			functionPointer = (NDDS_Transport_create_plugin)eProsimaGetProcAddress(libraryHandle, functionName);;

			if(functionPointer == NULL)
				printf("ERROR<%s>: Cannot load the function %s from library %s\n", METHOD_NAME, functionName, libraryName);
		}
		else
		{
			printf("ERROR<%s>: Cannot load the library %s\n", METHOD_NAME, libraryName);
		}
	}
	else
	{
		printf("ERROR<%s>: Bad parameters\n", METHOD_NAME);
	}

	return functionPointer;
}

NDDS_Transport_Plugin* loadTransportUDPv4(const struct DDS_PropertyQosPolicy *property_in)
{
	const char* const METHOD_NAME = "loadTransportUDPv4";
    char pluginName[] = "UDPv4";
	NDDS_Transport_Plugin *newPlugin = NULL;
    struct NDDS_Transport_UDPv4_Property_t propertyUDPv4 = NDDS_TRANSPORT_UDPV4_PROPERTY_DEFAULT;
	struct DDS_Property_t *auxProperty = NULL;
	int auxLength = 0;
	char *auxString = NULL;

	if(pluginName != NULL && property_in != NULL)
	{
        // Property UDPv4.send_socket_buffer_size
		auxLength = strlen(pluginName) + strlen(".send_socket_buffer_size") + 1; // "UDPv4.send_socket_buffer_size"
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".send_socket_buffer_size");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.send_socket_buffer_size"

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.send_socket_buffer_size) != 1 ||
                        propertyUDPv4.send_socket_buffer_size < propertyUDPv4.parent.message_size_max)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.send_socket_buffer_size\n", METHOD_NAME);
                    propertyUDPv4.send_socket_buffer_size = NDDS_TRANSPORT_UDPV4_MESSAGE_SIZE_MAX_DEFAULT;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.recv_socket_buffer_size
		auxLength = strlen(pluginName) + strlen(".recv_socket_buffer_size") + 1; // "UDPv4.recv_socket_buffer_size
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".recv_socket_buffer_size");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.recv_socket_buffer_size"

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.recv_socket_buffer_size) != 1 ||
                        propertyUDPv4.recv_socket_buffer_size < propertyUDPv4.parent.message_size_max)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.recv_socket_buffer_size\n", METHOD_NAME);
                    propertyUDPv4.recv_socket_buffer_size = NDDS_TRANSPORT_UDPV4_MESSAGE_SIZE_MAX_DEFAULT;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.unicast_enabled
		auxLength = strlen(pluginName) + strlen(".unicast_enabled") + 1; // "UDPv4.unicast_enabled
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".unicast_enabled");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.unicast_enabled

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.unicast_enabled) != 1 ||
                        propertyUDPv4.unicast_enabled < 0 ||
                        propertyUDPv4.unicast_enabled > 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.unicast_enabled\n", METHOD_NAME);
                    propertyUDPv4.unicast_enabled = 1;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.multicast_enabled
		auxLength = strlen(pluginName) + strlen(".multicast_enabled") + 1; // "UDPv4.multicast_enabled
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".multicast_enabled");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.multicast_enabled

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.multicast_enabled) != 1 ||
                        propertyUDPv4.multicast_enabled < 0 ||
                        propertyUDPv4.multicast_enabled > 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.multicast_enabled\n", METHOD_NAME);
                    propertyUDPv4.multicast_enabled = NDDS_TRANSPORT_UDPV4_USE_MULTICAST_DEFAULT;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.multicast_ttl
		auxLength = strlen(pluginName) + strlen(".multicast_ttl") + 1; // "UDPv4.multicast_ttl
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".multicast_ttl");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.multicast_ttl

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.multicast_ttl) != 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.multicast_ttl\n", METHOD_NAME);
                    propertyUDPv4.multicast_ttl = NDDS_TRANSPORT_UDPV4_MULTICAST_TTL_DEFAULT;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.multicast_loopback_disabled
		auxLength = strlen(pluginName) + strlen(".multicast_loopback_disabled") + 1; // "UDPv4.multicast_loopback_disabled
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".multicast_loopback_disabled");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.multicast_loopback_disabled

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.multicast_loopback_disabled) != 1 ||
                        propertyUDPv4.multicast_loopback_disabled < 0 ||
                        propertyUDPv4.multicast_loopback_disabled > 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.multicast_loopback_disabled\n", METHOD_NAME);
                    propertyUDPv4.multicast_loopback_disabled = 0;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.ignore_loopback_interface
		auxLength = strlen(pluginName) + strlen(".ignore_loopback_interface") + 1; // "UDPv4.ignore_loopback_interface
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".ignore_loopback_interface");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.ignore_loopback_interface

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.ignore_loopback_interface) != 1 ||
                        propertyUDPv4.ignore_loopback_interface < -1 ||
                        propertyUDPv4.ignore_loopback_interface > 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.ignore_loopback_interface\n", METHOD_NAME);
                    propertyUDPv4.ignore_loopback_interface = -1;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.ignore_nonrunning_interfaces
		auxLength = strlen(pluginName) + strlen(".ignore_nonrunning_interfaces") + 1; // "UDPv4.ignore_nonrunning_interfaces
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".ignore_nonrunning_interfaces");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.ignore_nonrunning_interfaces

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.ignore_nonrunning_interfaces) != 1 ||
                        propertyUDPv4.ignore_nonrunning_interfaces < 0 ||
                        propertyUDPv4.ignore_nonrunning_interfaces > 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.ignore_nonrunning_interfaces\n", METHOD_NAME);
                    propertyUDPv4.ignore_nonrunning_interfaces = 0;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.no_zero_copy
		auxLength = strlen(pluginName) + strlen(".no_zero_copy") + 1; // "UDPv4.no_zero_copy
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".no_zero_copy");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.no_zero_copy

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.no_zero_copy) != 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.no_zero_copy\n", METHOD_NAME);
                    propertyUDPv4.no_zero_copy = 0;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.send_blocking
		auxLength = strlen(pluginName) + strlen(".send_blocking") + 1; // "UDPv4.send_blocking
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".send_blocking");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.send_blocking

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.send_blocking) != 1 ||
                        propertyUDPv4.send_blocking < NDDS_TRANSPORT_UDPV4_BLOCKING_NEVER ||
                        propertyUDPv4.send_blocking > NDDS_TRANSPORT_UDPV4_BLOCKING_UNICAST_ONLY)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.send_blocking\n", METHOD_NAME);
                    propertyUDPv4.send_blocking = NDDS_TRANSPORT_UDPV4_BLOCKING_DEFAULT;
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.transport_priority_mask
		auxLength = strlen(pluginName) + strlen(".transport_priority_mask") + 1; // "UDPv4.transport_priority_mask
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".transport_priority_mask");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.transport_priority_mask

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "0x%X", &propertyUDPv4.transport_priority_mask) != 1)
                {
                    if(sscanf(auxProperty->value, "%d", &propertyUDPv4.transport_priority_mask) != 1)
                    {
                        printf("ERROR<%s>: Bad value for UDPv4.transport_priority_mask\n", METHOD_NAME);
                        propertyUDPv4.transport_priority_mask = 0;
                    }
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.transport_priority_mapping_low
		auxLength = strlen(pluginName) + strlen(".transport_priority_mapping_low") + 1; // "UDPv4.transport_priority_mapping_low
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".transport_priority_mapping_low");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.transport_priority_mapping_low

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "0x%X", &propertyUDPv4.transport_priority_mapping_low) != 1)
                {
                    if(sscanf(auxProperty->value, "%d", &propertyUDPv4.transport_priority_mapping_low) != 1)
                    {
                        printf("ERROR<%s>: Bad value for UDPv4.transport_priority_mapping_low\n", METHOD_NAME);
                        propertyUDPv4.transport_priority_mapping_low = 0;
                    }
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.transport_priority_mapping_high
		auxLength = strlen(pluginName) + strlen(".transport_priority_mapping_high") + 1; // "UDPv4.transport_priority_mapping_high
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".transport_priority_mapping_high");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.transport_priority_mapping_high

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "0x%X", &propertyUDPv4.transport_priority_mapping_high) != 1)
                {
                    if(sscanf(auxProperty->value, "%d", &propertyUDPv4.transport_priority_mapping_high) != 1)
                    {
                        printf("ERROR<%s>: Bad value for UDPv4.transport_priority_mapping_high\n", METHOD_NAME);
                        propertyUDPv4.transport_priority_mapping_high = 0xff;
                    }
                }
			}

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
        
        // Property UDPv4.send_ping
		auxLength = strlen(pluginName) + strlen(".send_ping") + 1; // "UDPv4.send_ping
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".send_ping");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.send_ping

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.send_ping) != 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.send_ping\n", METHOD_NAME);
                    propertyUDPv4.send_ping = 1;
                }
            }

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}

		// Property UDPv4.parent.message_size_max
		auxLength = strlen(pluginName) + strlen(".parent.message_size_max") + 1; // "UDPv4.parent.message_size_max
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".parent.message_size_max");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.message_size_max

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.parent.message_size_max) != 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.parent.message_size_max\n", METHOD_NAME);
                    propertyUDPv4.parent.message_size_max = 9216;
                }
            }

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}

		// Property UDPv4.parent.gather_send_buffer_count_max
		auxLength = strlen(pluginName) + strlen(".parent.gather_send_buffer_count_max") + 1; // "UDPv4.parent.gather_send_buffer_count_max
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".parent.gather_send_buffer_count_max");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.gather_send_buffer_count_max

			if(auxProperty != NULL)
			{
                if(sscanf(auxProperty->value, "%d", &propertyUDPv4.parent.gather_send_buffer_count_max) != 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.parent.gather_send_buffer_count_max\n", METHOD_NAME);
                    // TODO Get MIN define from RTI DDS
                    propertyUDPv4.parent.gather_send_buffer_count_max = 3;
                }
            }

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}

		// Property UDPv4.parent.allow_interfaces_list
		auxLength = strlen(pluginName) + strlen(".parent.allow_interfaces_list") + 1; // "UDPv4.parent.allow_interfaces_list
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".parent.allow_interfaces_list");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Value of "UDPv4.allow_interfaces_list

			if(auxProperty != NULL)
			{
				propertyUDPv4.parent.allow_interfaces_list_length = 1;
				RTIOsapiHeap_allocateArray(&propertyUDPv4.parent.allow_interfaces_list, 1, char*);
				auxLength = strlen(auxProperty->value) + 1;
				RTIOsapiHeap_allocateArray(propertyUDPv4.parent.allow_interfaces_list, auxLength, char);
                if(sscanf(auxProperty->value, "%s", propertyUDPv4.parent.allow_interfaces_list[0]) != 1)
                {
                    printf("ERROR<%s>: Bad value for UDPv4.parent.allow_interfaces_list\n", METHOD_NAME);
					RTIOsapiHeap_freeArray(propertyUDPv4.parent.allow_interfaces_list[0]);
					RTIOsapiHeap_freeArray(propertyUDPv4.parent.allow_interfaces_list);
					propertyUDPv4.parent.allow_interfaces_list = NULL;
					propertyUDPv4.parent.allow_interfaces_list_length = 0;
                }
            }

            DDS_String_free(auxString);
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}

        // Create the UDPv4 plugin.
        newPlugin = NDDS_Transport_UDPv4_new(&propertyUDPv4);
	}
	else
	{
		printf("ERROR<%s>: Bad parameters\n", METHOD_NAME);
	}

	return newPlugin;
}

NDDS_Transport_Plugin* loadTransportPluginFromLibrary(const char *pluginName, const struct DDS_PropertyQosPolicy *property_in,
													  NDDS_Transport_Address_t *default_network_address_out)
{
	const char* const METHOD_NAME = "loadTransportPluginFromLibrary";
	NDDS_Transport_Plugin *newPlugin = NULL;
	NDDS_Transport_create_plugin functionPointer = NULL;
	struct DDS_Property_t *auxProperty = NULL, *auxProperty2 = NULL;
	int auxLength = 0;
	char *auxString = NULL;

	if(pluginName != NULL && property_in != NULL)
	{
		auxLength = strlen(pluginName) + strlen(".library") + 1; // "pluginName.library"
		auxString = DDS_String_alloc(auxLength);

		if(auxString != NULL)
		{
			SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".library");
			auxProperty = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString); // Valur of "pluginName.library"

			if(auxProperty != NULL)
			{
				DDS_String_free(auxString);
				auxLength = strlen(pluginName) + strlen(".create_function") + 1; // "pluginName.create_function"
				auxString = DDS_String_alloc(auxLength);

				if(auxString != NULL)
				{
					SNPRINTF(auxString, auxLength, "%s%s", pluginName, ".create_function");
					auxProperty2 = DDS_PropertyQosPolicyHelper_lookup_property((struct DDS_PropertyQosPolicy*)property_in, auxString);

					if(auxProperty2 != NULL)
					{
						functionPointer = loadLibrary(auxProperty->value, auxProperty2->value);

						if(functionPointer != NULL)
						{
							struct DDS_PropertyQosPolicy newProperties;

							DDS_PropertyQosPolicy_initialize(&newProperties);
							getSubtransportProperties(property_in, &newProperties, pluginName);

							newPlugin = functionPointer(default_network_address_out, &newProperties); 

							DDS_PropertyQosPolicy_finalize(&newProperties);
						}
					}
					else
					{
						printf("ERROR<%s>: There is not defined %s\n", METHOD_NAME, auxString);
					}

					DDS_String_free(auxString);
				}
				else
				{
					printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
				}
			}
			else
			{
				printf("ERROR<%s>: There is not defined %s\n", METHOD_NAME, auxString);
			}
		}
		else
		{
			printf("ERROR<%s>: Cannot allocate memory to create string\n", METHOD_NAME);
		}
	}
	else
	{
		printf("ERROR<%s>: Bad parameters\n", METHOD_NAME);
	}

	return newPlugin;
}

void getSubtransportProperties(const struct DDS_PropertyQosPolicy *transportProperties, struct DDS_PropertyQosPolicy *subtransportProperties,
							   const char *subtransportName)
{
	struct DDS_PropertySeq property_seq;
	struct DDS_Property_t *auxProperty = NULL, *newProperty = NULL;
	int count = 0, transportPropertiesLength = 0, subtransportNameLength = 0;

	DDS_PropertyQosPolicyHelper_get_properties((struct DDS_PropertyQosPolicy*)transportProperties, &property_seq, subtransportName);
	transportPropertiesLength = DDS_PropertySeq_get_length(&property_seq);

	DDS_PropertySeq_ensure_length(&subtransportProperties->value, transportPropertiesLength,
		transportPropertiesLength);

	subtransportNameLength = strlen(subtransportName) + 1; // "pluginName."
	for(; count < transportPropertiesLength; count++)
	{
		auxProperty = DDS_PropertySeq_get_reference(&property_seq, count);

		if(auxProperty != NULL)
		{
			newProperty = DDS_PropertySeq_get_reference(&subtransportProperties->value, count);

			if(newProperty != NULL)
			{
				newProperty->name = DDS_String_dup(auxProperty->name + subtransportNameLength);
				newProperty->value = DDS_String_dup(auxProperty->value);
			}
		}
	}
}

void copyNDDSTransportProperties(struct NDDS_Transport_Property_t *dst, const struct NDDS_Transport_Property_t *src)
{
    const char* const METHOD_NAME = "copyNDDSTransportProperties";
    int i = 0;

    if(dst != NULL && src != NULL)
    {
        dst->classid = src->classid;
        dst->address_bit_count = src->address_bit_count;
        dst->properties_bitmap = src->properties_bitmap;
        dst->gather_send_buffer_count_max = src->gather_send_buffer_count_max;
        dst->message_size_max = src->message_size_max;
        dst->allow_interfaces_list_length = src->allow_interfaces_list_length;

        if(dst->allow_interfaces_list_length > 0)
        {
            RTIOsapiHeap_allocateArray(&dst->allow_interfaces_list,
                    dst->allow_interfaces_list_length, char*);

            for(i = 0; i < dst->allow_interfaces_list_length; ++i)
            {
                dst->allow_interfaces_list[i] = DDS_String_dup(src->allow_interfaces_list[i]);
            }

        }
        dst->deny_interfaces_list_length = src->deny_interfaces_list_length;

        if(dst->deny_interfaces_list_length > 0)
        {
            RTIOsapiHeap_allocateArray(&dst->deny_interfaces_list,
                    dst->deny_interfaces_list_length, char*);

            for(i = 0; i < dst->deny_interfaces_list_length; ++i)
            {
                dst->deny_interfaces_list[i] = DDS_String_dup(src->deny_interfaces_list[i]);
            }

        }

        dst->allow_multicast_interfaces_list_length = src->allow_multicast_interfaces_list_length;

        if(dst->allow_multicast_interfaces_list_length > 0)
        {
            RTIOsapiHeap_allocateArray(&dst->allow_multicast_interfaces_list,
                    dst->allow_multicast_interfaces_list_length, char*);

            for(i = 0; i < dst->allow_multicast_interfaces_list_length; ++i)
            {
                dst->allow_multicast_interfaces_list[i] = DDS_String_dup(src->allow_multicast_interfaces_list[i]);
            }

        }
        dst->deny_multicast_interfaces_list_length = src->deny_multicast_interfaces_list_length;

        if(dst->deny_multicast_interfaces_list_length > 0)
        {
            RTIOsapiHeap_allocateArray(&dst->deny_multicast_interfaces_list,
                    dst->deny_multicast_interfaces_list_length, char*);

            for(i = 0; i < dst->deny_multicast_interfaces_list_length; ++i)
            {
                dst->deny_multicast_interfaces_list[i] = DDS_String_dup(src->deny_multicast_interfaces_list[i]);
            }

        }
    }
    else
    {
        printf("ERROR<%s>: Bad parameters\n", METHOD_NAME);
    }
}

void finalizeNDDSTransportProperties(struct NDDS_Transport_Property_t *properties)
{
    const char* const METHOD_NAME = "finalizeNDDSTransportProperties";
    int i = 0;

    if(properties != NULL)
    {
        if(properties->allow_interfaces_list_length > 0)
        {   
            for(i = 0; i < properties->allow_interfaces_list_length; ++i)
            {
                DDS_String_free(properties->allow_interfaces_list[i]);
            }

            RTIOsapiHeap_freeArray(properties->allow_interfaces_list);
            properties->allow_interfaces_list = NULL;
            properties->allow_interfaces_list_length = 0;
        }

        if(properties->deny_interfaces_list_length > 0)
        {            
            for(i = 0; i < properties->deny_interfaces_list_length; ++i)
            {
                DDS_String_free(properties->deny_interfaces_list[i]);
            }

            RTIOsapiHeap_freeArray(properties->deny_interfaces_list);
            properties->deny_interfaces_list = NULL;
            properties->deny_interfaces_list_length = 0;
        }

        if(properties->allow_multicast_interfaces_list_length > 0)
        {
            for(i = 0; i < properties->allow_multicast_interfaces_list_length; ++i)
            {
                DDS_String_free(properties->allow_multicast_interfaces_list[i]);
            }

            RTIOsapiHeap_freeArray(properties->allow_multicast_interfaces_list);
            properties->allow_multicast_interfaces_list = NULL;
            properties->allow_multicast_interfaces_list_length = 0;
        }

        if(properties->deny_multicast_interfaces_list_length > 0)
        {
            for(i = 0; i < properties->deny_multicast_interfaces_list_length; ++i)
            {
                DDS_String_free(properties->deny_multicast_interfaces_list[i]);
            }

            RTIOsapiHeap_freeArray(properties->deny_multicast_interfaces_list);
            properties->deny_multicast_interfaces_list = NULL;
            properties->deny_multicast_interfaces_list_length = 0;
        }
    }
    else
    {
        printf("ERROR<%s>: Bad parameters\n", METHOD_NAME);
    }
}
