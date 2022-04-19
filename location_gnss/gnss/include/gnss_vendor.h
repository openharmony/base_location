/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_GNSS_VENDOR_H
#define OHOS_GNSS_VENDOR_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
namespace OHOS {
namespace Location {
#define SV_NUM_MAX 64

/** Milliseconds since January 1, 1970 */
typedef int64_t GnssUtcTimestamp;

typedef uint16_t AGnssSetIDClass;
enum {
    AGNSS_SETID_CLASS_NONE = 0,
    AGNSS_SETID_CLASS_IMSI = 1,
    AGNSS_SETID_CLASS_MSISDM = 2,
};

typedef uint32_t AGnssRefLocClass;
enum {
    AGNSS_REF_LOC_CLASS_CELLID = 1,
    AGNSS_REF_LOC_CLASS_MAC = 2,
};

typedef uint32_t GnssStartClass;
enum {
    GNSS_START_CLASS_NORMAL = 1,
    GNSS_START_CLASS_GNSS_CACHE = 2,
};

typedef uint16_t CellIdClass;
enum {
    GSM_CELLID   = 1,
    UMTS_CELLID  = 2,
    LTE_CELLID   = 3,
};

typedef uint16_t ApnIpClass;
enum {
    APN_CLASS_INVALID  = 0,
    APN_CLASS_IPV4     = 1,
    APN_CLASS_IPV6     = 2,
    APN_CLASS_IPV4V6   = 3
};

typedef uint16_t AGnssDataConnStatus;
enum  {
    /** AGNSS data connecting. */
    AGNSS_DATA_CONN_CONNECTING  = 1,
    /** AGNSS data connection initiated. */
    AGNSS_DATA_CONN_CONNECTED     = 2,
    /** AGNSS data disconnecting. */
    AGNSS_DATA_CONN_DISCONNECTING  = 3,
    /** AGNSS data disconnected. */
    AGNSS_DATA_CONN_DISCONNECTED   = 4
};

typedef uint16_t AGnssClass;
enum {
    AGNSS_CLASS_SUPL         = 1,
    AGNSS_CLASS_C2K          = 2
};

/**
 * Constellation class
 */
typedef uint8_t ConstellationClass;
enum {
    GNSS_CONSTELLATION_UNKNOWN = 0,
    /** Global Positioning System. */
    GNSS_CONSTELLATION_GPS     = 1,
    /** Satellite-Based Augmentation System. */
    GNSS_CONSTELLATION_SBAS    = 2,
    /** Global Navigation Satellite System. */
    GNSS_CONSTELLATION_GLONASS = 3,
    /** Quasi-Zenith Satellite System. */
    GNSS_CONSTELLATION_QZSS    = 4,
    /** BeiDou Navigation Satellite System. */
    GNSS_CONSTELLATION_BEIDOU  = 5,
    /** Galileo Navigation Satellite System. */
    GNSS_CONSTELLATION_GALILEO = 6,
    /** Indian Regional Navigation Satellite System. */
    GNSS_CONSTELLATION_IRNSS   = 7,
};

/** GNSS working status values. */
typedef uint16_t GnssWorkingStatus;
enum {
    /** GNSS status unknown. */
    GNSS_STATUS_NONE           = 0,
    /** GNSS has begun navigating. */
    GNSS_STATUS_SESSION_BEGIN  = 1,
    /** GNSS has stopped navigating. */
    GNSS_STATUS_SESSION_END    = 2,
    /** GNSS has powered on but is not navigating. */
    GNSS_STATUS_ENGINE_ON      = 3,
    /** GNSS is powered off. */
    GNSS_STATUS_ENGINE_OFF     = 4
};

typedef uint32_t GnssCapabilities;
enum {
    /** GNSS supports MS-Based AGNSS mode */
    GNSS_CAP_SUPPORT_MSB                             = (1 << 0),
    /** GNSS supports MS-Assisted AGNSS mode */
    GNSS_CAP_SUPPORT_MSA                             = (1 << 1),
    /** GNSS supports Geofencing  */
    GNSS_CAP_SUPPORT_GEOFENCING                      = (1 << 2),
    /** GNSS supports Measurements for at least GPS. */
    GNSS_CAP_SUPPORT_MEASUREMENTS                    = (1 << 3),
    /** GNSS supports Navigation Messages */
    GNSS_CAP_SUPPORT_NAV_MESSAGES                    = (1 << 4),
    /** GNSS supports location caching */
    GNSS_CAP_SUPPORT_GNSS_CACHE                    = (1 << 5),
};

typedef uint32_t SatellitesStatusFlag;
enum {
    SATELLITES_STATUS_NONE                  = 0,
    SATELLITES_STATUS_HAS_EPHEMERIS_DATA    = 1 << 0,
    SATELLITES_STATUS_HAS_ALMANAC_DATA      = 1 << 1,
    SATELLITES_STATUS_USED_IN_FIX           = 1 << 2,
    SATELLITES_STATUS_HAS_CARRIER_FREQUENCY = 1 << 3
};

typedef int GnssWorkingMode;
enum {
    GNSS_WORKING_MODE_STANDALONE = 1, // GNSS standalone (no assistance)
    GNSS_WORKING_MODE_MS_BASED = 2, // AGNSS MS-Based mode
    GNSS_WORKING_MODE_MS_ASSISTED = 3 // AGPS MS-Assisted mode
};

typedef int GnssRefInfoClass;
enum {
    GNSS_REF_INFO_TIME = 1,
    GNSS_REF_INFO_LOCATION = 2,
    GNSS_REF_INFO_BEST_LOCATION = 3,
};

typedef uint16_t GnssAuxiliaryDataClass;
enum {
    GNSS_AUXILIARY_DATA_CLASS_EPHEMERIS    = 0x0001,
    GNSS_AUXILIARY_DATA_CLASS_ALMANAC      = 0x0002,
    GNSS_AUXILIARY_DATA_CLASS_POSITION     = 0x0004,
    GNSS_AUXILIARY_DATA_CLASS_TIME         = 0x0008,
    GNSS_AUXILIARY_DATA_CLASS_IONO         = 0x0010,
    GNSS_AUXILIARY_DATA_CLASS_UTC          = 0x0020,
    GNSS_AUXILIARY_DATA_CLASS_HEALTH       = 0x0040,
    GNSS_AUXILIARY_DATA_CLASS_SVDIR        = 0x0080,
    GNSS_AUXILIARY_DATA_CLASS_SVSTEER      = 0x0100,
    GNSS_AUXILIARY_DATA_CLASS_SADATA       = 0x0200,
    GNSS_AUXILIARY_DATA_CLASS_RTI          = 0x0400,
    GNSS_AUXILIARY_DATA_CLASS_CELLDB_INFO  = 0x8000,
    GNSS_AUXILIARY_DATA_CLASS_ALL          = 0xFFFF
};

typedef int GnssModuleIfaceClass;
enum {
    AGPS_INTERFACE = 1,
    GNSS_GEOFENCING_INTERFACE = 2,
    GNSS_CACHE_INTERFACE = 3,
};

typedef int32_t GeofenceEvent;
enum {
    GEOFENCE_EVENT_UNCERTAIN  = (1 << 0),
    GEOFENCE_EVENT_ENTERED    = (1 << 1),
    GEOFENCE_EVENT_EXITED    = (1 << 2),
};

typedef int32_t GeofenceOperateClass;
enum {
    GEOFENCE_ADD    = 1,
    GEOFENCE_DELETE = 2,
};

typedef int32_t GeofenceOperateResult;
enum {
    GEOFENCE_OPERATION_SUCCESS        = 0,
    GEOFENCE_OPERATION_ERROR_UNKNOWN            = -100,
    GEOFENCE_OPERATION_ERROR_TOO_MANY_GEOFENCES = -101,
    GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_EXISTS = -102,
    GEOFENCE_OPERATION_ERROR_PARAMS_INVALID     = -103,
};

/* CellID info struct. */
typedef struct {
    size_t size;

    CellIdClass type;
    /** Mobile Country Code. */
    uint16_t mcc;
    /** Mobile Network Code. */
    uint16_t mnc;
    /** Location Area Code in 2G, 3G and LTE. */
    uint16_t lac;
    /** 2G:Cell id. 3G:Utran Cell id. LTE:Cell Global Id EUTRA. */
    uint32_t cid;
    /** Tracking Area Code in LTE. */
    uint16_t tac;
    /** Physical Cell id in LTE. */
    uint16_t pcid;
} AGnssRefInfoCellId;

typedef struct {
    size_t size;
    uint8_t mac[6];
} AGnssRefInfoMac;

/** Agnss reference location information structure */
typedef struct {
    size_t size;
    AGnssRefLocClass type;
    union {
        AGnssRefInfoCellId   cellId;
        AGnssRefInfoMac      mac;
    } u;
} AGnssRefLocInfo;

/** GNSS position structure. */
typedef struct {
    size_t size;
    uint32_t flags;
    double          latitude;
    double          longitude;
    /** Altitude in meters. */
    double          altitude;
    /** Speed in meters per second. */
    float           speed;
    /** Heading in degrees. */
    float           bearing;
    /**
     * Represents expected horizontal position accuracy, radial, in meters
     * (68% confidence).
     */
    float           horizontal_accuracy;
    /**
     * Represents expected vertical position accuracy in meters
     * (68% confidence).
     */
    float           vertical_accuracy;
    /**
     * Represents expected speed accuracy in meter per seconds
     * (68% confidence).
     */
    float           speed_accuracy;
    /**
     * Represents expected bearing accuracy in degrees
     * (68% confidence).
     */
    float           bearing_accuracy;
    /** Timestamp for the location fix. */
    GnssUtcTimestamp      timestamp;
    /** Timestamp since boot. */
    GnssUtcTimestamp      timestamp_since_boot;
} GnssLocation;

typedef struct {
    size_t size;
    /** min interval between locations in ms. */
    uint32_t  interval;
    /** If the value is true, the cached locations
     *  are reported and the AP is woken up after the FIFO is full.
     */
    bool      fifoFullNotify;
} GnssCachingConfig;

/**
 * Represents Satellite Statu info.
 */
typedef struct {
    size_t size;
    /**
     * Pseudo-random or satellite ID number for the satellite, a.k.a. Space Vehicle (SV), or
     * FCN/OSN number for Glonass. The distinction is made by looking at constellation field.
     * Values must be in the range of:
     *
     * - GNSS:    1-32
     * - SBAS:    120-151, 183-192
     * - GLONASS: 1-24, the orbital slot number (OSN), if known.  Or, if not:
     *            93-106, the frequency channel number (FCN) (-7 to +6) offset by
     *            + 100
     *            i.e. report an FCN of -7 as 93, FCN of 0 as 100, and FCN of +6
     *            as 106.
     * - QZSS:    193-200
     * - Galileo: 1-36
     * - Beidou:  1-37
     * - IRNSS:   1-14
     */
    int16_t satellite_id;

    /** Defines the constellation type. */
    ConstellationClass constellationType;

    /** Carrier-to-noise density in dB-Hz */
    float cn0;

    /** Elevation of SV in degrees. */
    float elevation;

    /** Azimuth of SV in degrees. */
    float azimuth;

    /** Carrier frequency of the signal tracked. */
    float carrier_frequencie;

    SatellitesStatusFlag flag;
} SatelliteStatusInfo;

/**
 * Represents all satellite status info.
 */
typedef struct {
    /** set to sizeof(GnssSatelliteStatus) */
    size_t size;

    /** Number of GNSS SVs currently visible. */
    uint32_t satellites_num;

    /** Pointer to an array of SVs information for all GNSS constellations. */
    SatelliteStatusInfo satellites_list[SV_NUM_MAX];
} GnssSatelliteStatus;

/**  Callback with location information. */
typedef void (* on_location_change)(GnssLocation* location);

/** Callback with gnss working status information. */
typedef void (* on_gnss_status_change)(GnssWorkingStatus* status);

/** Callback with satellite status information. */
typedef void (* on_sv_status_change)(GnssSatelliteStatus* status);

/** Callback for reporting NMEA info. */
typedef void (* on_gnss_nmea_change)(GnssUtcTimestamp timestamp, const char* nmea, int length);

/** Callback to reporting the GNSS capabilities. */
typedef void (* on_capabilities_change)(GnssCapabilities capabilities);

/** Request Delivery Reference Information. */
typedef void (* request_reference_information)(GnssRefInfoClass type);

/** Gnss basic config structure. */
typedef struct {
    size_t size;
    uint32_t min_interval; // min interval between locations in ms
    GnssWorkingMode gnssMode;
} GnssBasicConfigPara;

/** GNSS config structure. */
typedef struct {
    size_t size;
    GnssStartClass type;
    union {
        GnssBasicConfigPara   gnss_basic_config;
        GnssCachingConfig      gnss_cache_config;
    } u;
} GnssConfigPara;

/** Gnss reference time. */
typedef struct {
    size_t size;
    GnssUtcTimestamp time;
    GnssUtcTimestamp timeReference;
    int uncertainty;
} GnssRefTime;

/** Gnss reference position. */
typedef struct {
    size_t size;
    double latitude;
    double longitude;
    float accuracy;
} GnssRefLocation;

/** Gnss reference information structure. */
typedef struct {
    size_t size;
    GnssRefInfoClass type;
    union {
        GnssRefTime time;
        GnssRefLocation location;
        GnssLocation best_location;
    } u;
} GnssRefInfo;

/** Callback to request the client to download XTRA data. */
typedef void (* extended_ephemeris_download_request)();

/** GNSS cache location information reporting. */
typedef void (* on_cached_locations_change)(const GnssLocation** location_array, size_t len);

/** GNSS basic callback functions. */
typedef struct {
    size_t size;
    on_location_change location_update;
    on_gnss_status_change status_update;
    on_sv_status_change sv_status_update;
    on_gnss_nmea_change nmea_update;
    on_capabilities_change capabilities_update;
    request_reference_information ref_info_request;
    extended_ephemeris_download_request download_request_cb;
} GnssBasicCallbackIfaces;

/** GNSS cache callback functions. */
typedef struct {
    size_t size;
    on_cached_locations_change cached_location_cb;
} GnssCacheCallbackIfaces;

/** GNSS callback structure. */
typedef struct {
    size_t size;
    GnssBasicCallbackIfaces   gnss_cb;
    GnssCacheCallbackIfaces      gnss_cache_cb;
} GnssCallbackStruct;

/** GNSS vendor interface definition. */
typedef struct {
    size_t size;

    /** Enable the GNSS function.Initializing the GNSS Chip. */
    int (* enable_gnss)(GnssCallbackStruct* callbacks);

    /** Disables the GNSS function. */
    int (* disable_gnss)(void);

    /** start navigating. */
    int (* start_gnss)(GnssStartClass type);

    /** Stops navigating */
    int (* stop_gnss)(GnssStartClass type);

    /** Inject reference information into the GNSS chip. */
    int (* injects_reference_information)(GnssRefInfoClass type, GnssRefInfo* info);

    /** Set gnss configuration parameters. */
    int (* set_gnss_config_para)(GnssConfigPara* para);

    /**
     * Specifies that the next call to start will not use the
     * information defined in the flags.
     */
    void (* remove_auxiliary_data)(GnssAuxiliaryDataClass flags);

    /** Injects XTRA data into the GNSS. */
    int (* inject_extended_ephemeris)(char* data, int length);

    /** Return the cached locations size. */
    int (* get_cached_locations_size)();

    /** Retrieve all cached locations currently stored and clear the buffer. */
    void (* flush_cached_gnss_locations)();

    /** Get a pointer to gnss module interface. */
    const void* (* get_gnss_module_iface)(GnssModuleIfaceClass iface);
} GnssVendorInterface;

struct GnssVendorDevice {
    size_t size;
    const GnssVendorInterface* (*get_gnss_interface)();
};

/* Status of AGNSS. */
typedef struct {
    size_t size;

    AGnssClass                agnss_type;
    AGnssDataConnStatus      conn_status;
    /** IPv4 address. */
    uint32_t                ipaddr;
    /** Contain the IPv4 (AF_INET) or IPv6 (AF_INET6) address to report. */
    struct sockaddr_storage sock_addr;
} AGnssStatusInfo;

typedef void (* on_agnss_status_change)(const AGnssStatusInfo* status);

typedef void (* get_setid_cb)(AGnssSetIDClass type);

typedef void (* get_ref_location_cb)(AGnssRefLocClass type);

typedef struct {
    size_t size;
    on_agnss_status_change agnss_status_change;
    get_setid_cb get_setid;
    get_ref_location_cb get_ref_loc;
} AGnssCallbackIfaces;

/** interface for AGNSS support */
typedef struct {
    size_t size;

    /**
     * Opens the AGNSS interface and provides the callback interfaces
     */
    bool (* set_agnss_callback)(AGnssCallbackIfaces* callbacks);

    /**
     * Sets the reference cell id.
     */
    bool (* set_ref_location)(const AGnssRefLocInfo* reg_loc);

    /**
     * Sets the set ID.
     */
    bool (* set_setid)(AGnssSetIDClass type, const char* setid, size_t len);

    /**
     * Setting the Agnss Server Information.
     */
    bool (* set_agnss_server)(AGnssClass type, const char* server, size_t len, int32_t port);
} AGnssModuleInterface;

/**
 * The callback associated with the geofence.
 * Parameters:
 *      geofence_id - The id associated with the add_gnss_geofence.
 *      location    - The current GNSS location.
 *      event  - Can be one of GEOFENCE_EVENT_UNCERTAIN, GEOFENCE_EVENT_ENTERED,
 *                    GEOFENCE_EVENT_EXITED.
 *      timestamp   - Timestamp when the transition was detected.
 */
typedef void (* geofence_event_callback)(int32_t geofence_id,  GnssLocation* location,
    GeofenceEvent event, GnssUtcTimestamp timestamp);

/**
 * Callback function that indicates whether the geofence service is available.
 *
 * Parameters:
 *  is_available is true when gnss geofence service is available.
 */
typedef void (* geofence_availability_callback)(bool is_available);

/**
 * Callback function indicating the result of the geofence operation
 *
 * geofence_id - Id of the geofence.
 * operate_type - geofence operate type.
 * result - GEOFENCE_OPERATION_SUCCESS
 *          GEOFENCE_OPERATION_ERROR_TOO_MANY_GEOFENCES  - geofence limit has been reached.
 *          GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_EXISTS  - geofence with id already exists
 *          GEOFENCE_OPERATION_ERROR_PARAMS_INVALID - input params are invalid.
 */
typedef void (* geofence_operate_result_callback)(int32_t geofence_id, GeofenceOperateClass operate_type,
    GeofenceOperateResult result);

typedef struct {
    size_t size;
    geofence_availability_callback on_geofence_availability_change;
    geofence_event_callback geofence_event_notify;
    geofence_operate_result_callback geofence_operate_result_cb;
} GeofenceCallbackIfaces;

/** Interface for GNSS Geofence */
typedef struct {
    size_t size;

    /**
     * Opens the geofence interface and provides the callback interfaces.
     */
    bool (* set_callback)(GeofenceCallbackIfaces* callbacks);

    /**
     * Add a geofence area. This api currently supports circular geofences.
     * Parameters:
     *    geofence_id - The id for the geofence.
     *    latitude, longtitude, radius_meters - The lat, long and radius
     *       (in meters) for the geofence
     *    monitor_event - Which transitions to monitor. Bitwise OR of
     *       GEOFENCE_EVENT_UNCERTAIN, GEOFENCE_EVENT_ENTERED and
     *       GEOFENCE_EVENT_EXITED.
     */
    bool (* add_gnss_geofence)(int32_t geofence_id, double latitude, double longitude,
       double radius, GeofenceEvent monitor_event);

    /**
     * Remove a gnss geofence.
     * geofence_id - The id for the geofence.
     * Return true if delete successful.
     */
    bool (* delete_gnss_geofence)(int32_t geofence_id);
} GeofenceModuleInterface;
} // namespace Location
} // namespace OHOS
#endif /* OHOS_GNSS_VENDOR_H */
