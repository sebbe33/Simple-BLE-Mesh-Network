/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package simpleblemeshnetwork.exampleapp.network;

import java.util.HashMap;
import java.util.UUID;


public class MeshGattAttributes {
    private static HashMap<String, String> attributes = new HashMap<String, String>();
    /*public static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";
    public static String LEAVE_GROUP_UUID = "713d0005-503e-4c75-ba94-3148f18d941e";
    public static String JOIN_GROUP_UUID = "713d0004-503e-4c75-ba94-3148f18d941e";
    public static String RX_MESSAGE_UUID = "713d0003-503e-4c75-ba94-3148f18d941e";
    public static String TX_MESSAGE_UUID = "713d0002-503e-4c75-ba94-3148f18d941e";
    public static String MESH_SERV_UUID = "713d0000-503e-4c75-ba94-3148f18d941e";*/

    public final static UUID UUID_MESH_TX = UUID.fromString("713d0002-503e-4c75-ba94-3148f18d941e");
    public final static UUID UUID_MESH_RX = UUID.fromString("713d0003-503e-4c75-ba94-3148f18d941e");
    public final static UUID UUID_MESH_JOIN_GROUP = UUID.fromString(
            "713d0004-503e-4c75-ba94-3148f18d941e");
    public final static UUID UUID_MESH_LEAVE_GROUP = UUID.fromString(
            "713d0005-503e-4c75-ba94-3148f18d941e");
    public final static UUID UUID_MESH_NETWORK_NAME = UUID.fromString(
            "713d0007-503e-4c75-ba94-3148f18d941e");
    public final static UUID CLIENT_CHARACTERISTIC_CONFIG = UUID.fromString(
            "00002902-0000-1000-8000-00805f9b34fb");
    public final static UUID UUID_MESH_SERVICE = UUID
            .fromString("713d0000-503e-4c75-ba94-3148f18d941e");

    static {
        // Mesh Services.
        /*attributes.put(MESH_SERV_UUID, "BLE Shield Service");
        // RBL Characteristics.
        attributes.put(TX_MESSAGE_UUID, "BLE Shield TX");
        attributes.put(RX_MESSAGE_UUID, "BLE Shield RX");*/
    }

    public static String lookup(String uuid, String defaultName) {
        String name = attributes.get(uuid);
        return name == null ? defaultName : name;
    }
}