/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
loadLibrary("location_ani")

export class BusinessError<T = void> {
    code: number;
    data: T;
}

type AsyncCallback<T, E = void> = (data: T, err:BusinessError<E>) => void;

class Result<T, E>{
    result: T;
    error: E;
    constructor(result: T, error: E) {
        this.result = result;
        this.error = error;
    }
}

enum LocationSourceType {
    GNSS = 1,
    NETWORK = 2,
    INDOOR = 3,
    RTK = 4
}

enum LocationRequestPriority {
    UNSET = 0x200,
    ACCURACY,
    LOW_POWER,
    FIRST_FIX
}
enum LocationRequestScenario {
    UNSET = 0x300,
    NAVIGATION,
    TRAJECTORY_TRACKING,
    CAR_HAILING,
    DAILY_LIFE_SERVICE,
    NO_POWER
}
enum LocatingPriority {
    PRIORITY_ACCURACY = 0x501,
    PRIORITY_LOCATING_SPEED = 0x502
}

interface Location {
    latitude: double;
    longitude: double;
    altitude: double;
    accuracy: double;
    speed: double;
    timeStamp: long;
    direction: double;
    timeSinceBoot: long;
}

interface CurrentLocationRequest {
    priority?: LocationRequestPriority;
    scenario?: LocationRequestScenario;
    maxAccuracy?: number;
    timeoutMs?: number;
}

interface SingleLocationRequest {
    locatingPriority: LocatingPriority;
    locatingTimeoutMs: number;
}

class LocationInner implements Location {
    latitude: double = 0;
    longitude: double = 0;
    altitude: double = 0;
    accuracy: double = 0;
    speed: double = 0;
    timeStamp: long = 0;
    direction: double = 0;
    timeSinceBoot: long = 0;
}

type LocationRequestConfig = CurrentLocationRequest | SingleLocationRequest
native function getCurrentLocation(request?: LocationRequestConfig):Location;
export class geolocationManager {
    getCurrentLocation(request?: LocationRequestConfig):Promise<Location> {
        let p = new Promise<Location>((resolve: (v: Location) => void, reject :(error: Object) => void) =>{
            let p1 = taskpool.execute(getCurrentLocation);
            p1.then(e: NullishType)=>{
                let r : Location = e as Location;
                resolve(r);
            }
        });
        return p;
    }
    getCurrentLocation(request: LocationRequestConfig, callback:AsyncCallback<Location>):void {
        let p1 = taskpool.execute(getCurrentLocation);
        p1.then((e: NullishType)=>{
            let location : Location = e as Location;
            let err : BusinessError = new BusinessError;
            callback(err, location);
        });
    }
    getCurrentLocation(callback:AsyncCallback<Location>):void {
        let p1 = taskpool.execute(getCurrentLocation);
        p1.then((e: NullishType)=>{
            let location : Location = e as Location;
            let err : BusinessError = new BusinessError;
            callback(err, location);
        });
    }
}