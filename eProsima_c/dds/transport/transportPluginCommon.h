#ifndef _EPROSIMA_C_DDS_TRANSPORT_TRANSPORTPLUGINCOMMON_H_
#define _EPROSIMA_C_DDS_TRANSPORT_TRANSPORTPLUGINCOMMON_H_

#include <stdio.h>
#include "transport/transport_interface.h"
#include "ndds/ndds_transport_c.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

struct DDS_PropertyQosPolicy;

/**
 * \brief This function loads a dll library and gets the pointer of a function.
 *
 * The function is searched by name and this function has to be certain structure:
 *     typedef NDDS_Transport_Plugin* (*NDDS_Transport_create_plugin)
 *         (NDDS_Transport_Address_t *default_network_address_out,
 *         const struct DDS_PropertyQosPolicy *property_in);
 *
 * \param libraryName The name of the dll library. Cannot be NULL.
 * \param functionName The name of the function to be returned. Cannot be NULL.
 * \return A pointer to the function that was specified in the parameters. In error case,
 * NULL value is returned.
 */
NDDS_Transport_create_plugin loadLibrary(const char *libraryName,
        const char *functionName);

/**
 * \brief This function loads a UDPv4 subtransport. This function also gets UDPv4 subtransport properties from Qos profiles XML.
 * If the load operation was successfull, then this function creates the IDPv4 subtransport.
 *
 * \param property_in Properties of the transport that wants to create the UDPv4 subtransport. Cannot be NULL.
 * The user must to free all structure.
 * \return The new UDPv4 transporrt. In error case, NULL value is returned.
 */
NDDS_Transport_Plugin* loadTransportUDPv4(const struct DDS_PropertyQosPolicy *property_in);

/**
 * \brief This function loads a library where is the subtransport that is needed.
 * If the load operation was successfull, then this function creates the subtransport.
 *
 * \param pluginName The name of the subtransport. Cannot be NULL:
 * \param property_in Properties of the transport that wants to create the subtransport. Cannot be NULL.
 * At least they have to be the properties library and create_function. The user must to free all structure.
 * \return The new transporrt plugin. In error case, NULL value is returned.
 */
NDDS_Transport_Plugin* loadTransportPluginFromLibrary(const char *pluginName, const struct DDS_PropertyQosPolicy *property_in,
    NDDS_Transport_Address_t *default_network_address_out);

void copyNDDSTransportProperties(struct NDDS_Transport_Property_t *dst, const struct NDDS_Transport_Property_t *src);
void finalizeNDDSTransportProperties(struct NDDS_Transport_Property_t *properties);

/**
 * \brief logs a text message
 *
 * \param text a NULL terminated string.
 */
void log_debug(const char *text);

/**
 * \brief logs a formatted text message with printf syntax.
 *
 * \param format a NULL terminated string.
 * \param ...    values for param substitution.
 */
void log_debugf(const char *format, ...);

/**
 * \brief logs a binary buffer in hexadecimal format.
 *
 * \param text an optional NULL terminated string printed a line before the buffer dump.
 * \param buf a binary buffer.
 * \param len buffer length.
 * \param bytesPerLine bytes printed per line. Default 16.
 */
void log_hexdump(const char *text, const char *buf, int len, int bytesPerLine);

/**
 * \brief Logs a RTI transport Address.
 *
 * \param address
 */
void log_address(const NDDS_Transport_Address_t *address);

/**
 * \brief Logs a RTPS Message which may be splitted in several buffers.
 *
 * \param text an optional NULL terminated string printed a line before the RTPS Message dump.
 * \param buffer_in array of binary transport buffers.
 * \param count_in buffer array length.
 * \param bytesPerLine bytes printed per line. Default 16.
 */
void log_rtps_message(const char *text, const NDDS_Transport_Buffer_t buffer_in[], RTI_INT32 buffer_count_in, int bytesPerLine);

/**
 * \brief This function initializes the log system.
 *
 * \param fileName Indicates the file name where the log will be stored. If the value is NULL,
 * then the log will be shown in the standard output.
 */
void log_init(const char *fileName);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _EPROSIMA_C_DDS_TRANSPORT_TRANSPORTPLUGINCOMMON_H_
