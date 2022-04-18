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
/**
 * The id of this module
 */
#define GPS_HARDWARE_MODULE_ID "gnss"

#define SV_NUM_MAX 64

/** Milliseconds since January 1, 1970 */
typedef int64_t GnssUtcTimestamp;

typedef uint16_t AGnssSetIDType;
enum {
    ANSS_SETID_TYPE_NONE = 0,
    ANSS_SETID_TYPE_IMSI = 1,
    ANSS_SETID_TYPE_MSISDM = 2,
};

typedef uint32_t AGnssRefLocType;
enum {
    ANSS_REF_LOC_TYPE_CELLID = 1,
    ANSS_REF_LOC_TYPE_MAC = 2,
};

typedef uint32_t GnssStartType;
enum {
    GNSS_START_TYPE_NORMAL = 1,
    GNSS_START_TYPE_GNSS_CACHE = 2,
};

typedef uint16_t CellIdType;
enum {
    GSM_CELLID   = 1,
    UMTS_CELLID  = 2,
    LTE_CELLID   = 3,
};

typedef uint16_t ApnIpType;
enum {
    APN_TYPE_INVALID  = 0,
    APN_TYPE_IPV4     = 1,
    APN_TYPE_IPV6     = 2,
    APN_TYPE_IPV4V6   = 3
};

typedef uint16_t AGnssDataConnStatus;
enum  {
    /** AGNSS data connecting . */
    AGNSS_DATA_CONN_CONNECTING  = 1,
    /** AGNSS data connection initiated */
    AGNSS_DATA_CONN_CONNECTED     = 2,
    /** AGNSS data disconnecting. */
    AGNSS_DATA_CONN_DISCONNECTING  = 3,
    /** AGNSS data disconnected */
    AGNSS_DATA_CONN_DISCONNECTED   = 4
};

typedef uint16_t AGnssType;
enum {
    AGNSS_TYPE_SUPL         = 1,
    AGNSS_TYPE_C2K          = 2
};

/**
 * Constellation type of GnssSvInfo
 */
typedef uint8_t GnssConstellationType;
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

/** GNSS status event values. */
typedef uint16_t GnssStatus;
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

typedef int GnssWorkMode;
enum {
    GNSS_MODE_STANDALONE = 1, // GNSS standalone (no assistance)
    GNSS_MODE_MS_BASED = 2, // AGNSS MS-Based mode
    GNSS_MODE_MS_ASSISTED = 3 // AGPS MS-Assisted mode
};

typedef int GnssRefInfoType;
enum {
    GNSS_REF_INFO_TIME = 1,
    GNSS_REF_INFO_LOCATION = 2,
    GNSS_REF_INFO_BEST_LOCATION = 3,
};

typedef uint16_t GnssAidingData;
enum {
    GNSS_AIDING_DATA_EPHEMERIS    = 0x0001,
    GNSS_AIDING_DATA_ALMANAC      = 0x0002,
    GNSS_AIDING_DATA_POSITION     = 0x0004,
    GNSS_AIDING_DATA_TIME         = 0x0008,
    GNSS_AIDING_DATA_IONO         = 0x0010,
    GNSS_AIDING_DATA_UTC          = 0x0020,
    GNSS_AIDING_DATA_HEALTH       = 0x0040,
    GNSS_AIDING_DATA_SVDIR        = 0x0080,
    GNSS_AIDING_DATA_SVSTEER      = 0x0100,
    GNSS_AIDING_DATA_SADATA       = 0x0200,
    GNSS_AIDING_DATA_RTI          = 0x0400,
    GNSS_AIDING_DATA_CELLDB_INFO  = 0x8000,
    GNSS_AIDING_DATA_ALL          = 0xFFFF
};

typedef int GnssIfaceType;
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

typedef int32_t GeofenceOperateType;
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

/* CellID for 2G, 3G and LTE, used in AGNSS. */
typedef struct {
    size_t size;

    CellIdType type;
    /** Mobile Country Code. */
    uint16_t mcc;
    /** Mobile Network Code. */
    uint16_t mnc;
    /** Location Area Code in 2G, 3G and LTE. In 3G lac is discarded. In LTE,
     * lac is populated with tac, to ensure that we don't break old clients that
     * might rely in the old (wrong) behavior.
     */
    uint16_t lac;
    /** Cell id in 2G. Utran Cell id in 3G. Cell Global Id EUTRA in LTE. */
    uint32_t cid;
    /** Tracking Area Code in LTE. */
    uint16_t tac;
    /** Physical Cell id in LTE (not used in 2G and 3G) */
    uint16_t pcid;
} AGnssRefCellId;

typedef struct {
    size_t size;
    uint8_t mac[6];
} AGnssRefMac;

/** Represents ref locations */
typedef struct {
    size_t size;
    AGnssRefLocType type;
    union {
        AGnssRefCellId   cellId;
        AGnssRefMac      mac;
    } u;
} AGnssRefLoc;

/** Represents a location. */
typedef struct {
    size_t size;
    uint32_t flags;
    /** Represents latitude in degrees. */
    double          latitude;
    /** Represents longitude in degrees. */
    double          longitude;
    /**
     * Represents altitude in meters above the WGS 84 reference ellipsoid.
     */
    double          altitude;
    /** Represents speed in meters per second. */
    float           speed;
    /** Represents heading in degrees. */
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
 * Represents Satellite Status.
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
    int16_t satellite_ids;

    /**
     * Defines the constellation of the given SV.
     */
    GnssConstellationType constellation;

    /**
     * Carrier-to-noise density in dB-Hz, typically in the range [0, 63].
     * It contains the measured C/N0 value for the signal at the antenna port.
     *
     * This is a mandatory value.
     */
    float cn0;

    /** Elevation of SV in degrees. */
    float elevation;

    /** Azimuth of SV in degrees. */
    float azimuths;

    /**
     * Carrier frequency of the signal tracked, for example it can be the
     * GPS central frequency for L1 = 1575.45 MHz, or L2 = 1227.60 MHz, L5 =
     * 1176.45 MHz, varying GLO channels, etc. If the field is zero, it is
     * the primary common use central frequency, e.g. L1 = 1575.45 MHz for
     * GPS.
     *
     * For an L1, L5 receiver tracking a satellite on L1 and L5 at the same
     * time, two GnssSvInfo structs must be reported for this same
     * satellite, in one of the structs, all the values related
     * to L1 must be filled, and in the other all of the values related to
     * L5 must be filled.
     *
     * If the data is available, svFlag must contain HAS_CARRIER_FREQUENCY.
     */
    float carrier_frequencies;

    SatellitesStatusFlag flags;
} SatelliteStatusInfo;

/**
 * Represents SV status.
 */
typedef struct {
    /** set to sizeof(GnssSvStatus) */
    size_t size;

    /** Number of GPS SVs currently visible, refers to the SVs stored in sv_list */
    uint32_t satellites_num;

    /**
     * Pointer to an array of SVs information for all GNSS constellations,
     * except GPS, which is reported using sv_list
     */
    SatelliteStatusInfo gnss_sv_list[SV_NUM_MAX];
} GnssSatelliteStatus;


/**
 * Callback with location information. Can only be called from a thread created
 * by create_thread_cb.
 */
typedef void (* on_location_change)(GnssLocation* location);
/**
 * Callback with status information. Can only be called from a thread created by
 * create_thread_cb.
 */
typedef void (* on_gnss_status_change)(GnssStatus* status);
/**
 * Callback with satellite status information.
 * Can only be called from a thread created by create_thread_cb.
 */
typedef void (* on_sv_status_change)(GnssSatelliteStatus* sv_info);

/**
 * Callback for reporting NMEA sentences. Can only be called from a thread
 * created by create_thread_cb.
 */
typedef void (* on_gnss_nmea_change)(GnssUtcTimestamp timestamp, const char* nmea, int length);

/**
 * Callback to inform framework of the GPS engine's capabilities. Capability
 * parameter is a bit field of GPS_CAPABILITY_* flags.
 */
typedef void (* on_capabilities_change)(GnssCapabilities capabilities);

typedef void (* request_reference_information)(GnssRefInfoType type);


/** Gnss config structure. */
typedef struct {
    size_t size;
    uint32_t min_interval; // min interval between locations in ms
    GnssWorkMode gnssMode;
} GnssNormalConfigPara;

/** GNSS config structure. */
typedef struct {
    size_t size;
    GnssStartType type;
    union {
        GnssNormalConfigPara   gnss_config;
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
    GnssRefInfoType type; // min interval between locations in ms
    union {
        GnssRefTime time;
        GnssRefLocation location;
        GnssLocation best_location;
    } u;
} GnssRefInfo;

/**
 * Callback to request the client to download XTRA data. The client should
 * download XTRA data and inject it by calling inject_xtra_data(). Can only be
 * called from a thread created by create_thread_cb.
 */
typedef void (* extended_ephemeris_download_request)();

typedef void (* on_cached_locations_change)(const GnssLocation** location_array, size_t len);


/** GNSS callback structure. */
typedef struct {
    size_t size;
    on_location_change location_update;
    on_gnss_status_change status_update;
    on_sv_status_change sv_status_update;
    on_gnss_nmea_change nmea_update;
    on_capabilities_change capabilities_update;
    request_reference_information ref_info_request;
    extended_ephemeris_download_request download_request_cb;
} GnssCallbacks;

/** GNSS cache callback structure. */
typedef struct {
    size_t size;
    on_cached_locations_change cached_location_cb;
} GnssCacheCallbacks;

/** GNSS callback structure. */
typedef struct {
    size_t size;
    GnssCallbacks   gnss_cb;
    GnssCacheCallbacks      gnss_cache_cb;
} GnssCallbackStruct;

/** Represents the standard GPS interface. */
typedef struct {
    size_t size;

    /**
     * Enable the GNSS function.Initializing the GNSS Chip.
     */
    int (* enable_gnss)(GnssCallbackStruct* callbacks);

    /** Disables the GNSS function. */
    int (* disable_gnss)(void);

    /**
     * Opens the interface and provides the callback routines
     * to the implementation of this interface and start navigating.
     */
    int (* start_gnss)(GnssStartType type);

    /** Stops navigating and delete callback. */
    int (* stop_gnss)(GnssStartType type);

    /** Inject reference information into the GNSS chip. */
    int (* injects_reference_information)(GnssRefInfoType type, GnssRefInfo* info);

    /**
     * Setting gnss configuration parameters.
     */
    int (* set_gnss_config_para)(GnssConfigPara* para);

    /**
     * Specifies that the next call to start will not use the
     * information defined in the flags. GNSS_AIDING_DATA_ALL is passed for
     * a cold start.
     */
    void (* delete_aiding_data)(GnssAidingData flags);

    /** Injects XTRA data into the GPS. */
    int (* inject_extended_ephemeris)(char* data, int length);

    /**
     * Return the cached locations size.
     */
    int (* get_cached_locations_size)();

    /**
     * Retrieve all batched locations currently stored and clear the buffer.
     * flp_location_callback MUST be called in response, even if there are
     * no locations to flush (in which case num_locations should be 0).
     * Subsequent calls to get_batched_location or flush_batched_locations
     * should not return any of the locations returned in this call.
     */
    void (* flush_cached_gnss_locations)();

    /** Get a pointer to gnss module interface. */
    const void* (* get_gnss_module_iface)(GnssIfaceType iface);
} GnssInterface;

struct GnssDevice {
    size_t size;
    const GnssInterface* (*get_gnss_interface)();
};

/*
 * Represents the status of AGPS augmented to support IPv4 and IPv6.
 */
typedef struct {
    size_t size;

    AGnssType                type;
    AGnssDataConnStatus      status;
    /**
     * Must be set to a valid IPv4 address if the field 'addr' contains an IPv4
     * address, or set to INADDR_NONE otherwise.
     */
    uint32_t                ipaddr;
    /**
     * Must contain the IPv4 (AF_INET) or IPv6 (AF_INET6) address to report.
     * Any other value of addr.ss_family will be rejected.
     */
    struct sockaddr_storage addr;
} AGnssStatus;

typedef void (* on_agnss_status_change)(const AGnssStatus* status);

typedef void (* get_setid_cb)(AGnssSetIDType type);

typedef void (* get_ref_location_cb)(AGnssRefLocType type);

typedef struct {
    size_t size;
    on_agnss_status_change agnss_status_change;
    get_setid_cb get_setid;
    get_ref_location_cb get_ref_loc;
} AGnssCallbacks;

/**
 * interface for AGNSS support
 */
typedef struct {
    size_t size;

    /**
     * Opens the AGNSS interface and provides the callback routines
     * to the implementation of this interface.
     */
    bool (* set_agnss_callback)(AGnssCallbacks* callbacks);

    /**
     * Sets the reference cell id.
     */
    bool (* set_ref_location)(const AGnssRefLoc* reg_loc);

    /**
     * Sets the set ID.
     */
    bool (* set_setid)(AGnssSetIDType type, const char* setid, size_t len);

    /**
     * Setting the Agnss Server Information.
     */
    bool (* set_agnss_server)(AGnssType type, const char* server, size_t len, int32_t port);
} AGnssInterface;

/**
 * The callback associated with the geofence.
 * Parameters:
 *      geofence_id - The id associated with the add_gnss_geofence.
 *      location    - The current GPS location.
 *      event  - Can be one of GEOFENCE_EVENT_UNCERTAIN, GEOFENCE_EVENT_ENTERED,
 *                    GEOFENCE_EVENT_EXITED.
 *      timestamp   - Timestamp when the transition was detected.
 *
 * The callback should only be called when the caller is interested in that
 * particular transition. For instance, if the caller is interested only in
 * ENTERED transition, then the callback should NOT be called with the EXITED
 * transition.
 *
 * IMPORTANT: If a transition is triggered resulting in this callback, the GPS
 * subsystem will wake up the application processor, if its in suspend state.
 */
typedef void (* geofence_event_callback)(int32_t geofence_id,  GnssLocation* location,
    GeofenceEvent event, GnssUtcTimestamp timestamp);

/**
 * The callback associated with the availability of the GNSS system for geofencing
 * monitoring.
 *
 * Parameters:
 *  is_available is true when gnss geofence service is available.
 */
typedef void (* geofence_availability_callback)(bool is_available);

/**
 * The callback associated with the add_gnss_geofence/delete_gnss_geofence call.
 *
 * geofence_id - Id of the geofence.
 * operate_type - geofence operate type.
 * result - GEOFENCE_OPERATION_SUCCESS
 *          GEOFENCE_OPERATION_ERROR_TOO_MANY_GEOFENCES  - geofence limit has been reached.
 *          GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_EXISTS  - geofence with id already exists
 *          GEOFENCE_OPERATION_ERROR_PARAMS_INVALID - input params are invalid.
 */
typedef void (* geofence_operate_result_callback)(int32_t geofence_id, GeofenceOperateType operate_type,
    GeofenceOperateResult result);

typedef struct {
    size_t size;
    geofence_availability_callback on_geofence_availability_change;
    geofence_event_callback geofence_event_notify;
    geofence_operate_result_callback geofence_operate_result_cb;
} GeofenceCallbacks;

/** Interface for GNSS Geofence */
typedef struct {
    size_t size;

    /**
     * Opens the geofence interface and provides the callback routines
     * to the implementation of this interface.
     */
    bool (* set_callback)(GeofenceCallbacks* callbacks);

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
     * Remove a gnss geofence. After the function returns, no notifications
     * should be sent.
     * geofence_id - The id for the geofence.
     * Return true if delete successful.
     */
    bool (* delete_gnss_geofence)(int32_t geofence_id);
} GnssGeofenceInterface;
} // namespace Location
} // namespace OHOS
#endif /* OHOS_GNSS_VENDOR_H */
