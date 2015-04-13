package simpleblemeshnetwork.exampleapp.network.application;

import android.bluetooth.BluetoothGattCharacteristic;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

import simpleblemeshnetwork.exampleapp.network.MeshGattAttributes;
import simpleblemeshnetwork.exampleapp.network.connection.Connection;
import simpleblemeshnetwork.exampleapp.network.connection.MeshBluetoothService;
import simpleblemeshnetwork.exampleapp.network.connection.MeshCharacteristicUpdatedCallback;

/**
 * @author Sebastian Blomberg, Markus Andersson
 */
public class MeshMessageManagerImpl implements MeshMessageManager {
    private final MeshBluetoothService btService;
    private Connection connection;
    /** A hash map to map an application's ID to a set of callback objects for that app */
    private final HashMap<MeshNodeApplication, Set<MeshMessageCallback>> callbacks = new HashMap<>();


    public MeshMessageManagerImpl(MeshBluetoothService btService, Connection connection) {
        this.btService = btService;
        this.connection = connection;
        // Register a callback with the bluetooth service for the TX char
        this.btService.registerCharacteristicUpdateCallback(characteristicUpdatedCallback);
    }


    private void sendMessageHelper(MessageType type, short destination,
                                   MeshNodeApplication targetApplication, byte[] message) {
        if(!connection.isActive()) {
            return;
        }

        BluetoothGattCharacteristic rx = btService.getSupportedGattService().
                getCharacteristic(MeshGattAttributes.UUID_MESH_RX);
        // Length = length(2) + type(1) + destination(2) + applicationId(2) + message.length
        byte length = (byte) (1 + message.length);

        // Build the data msg to be sent
        ByteBuffer b = ByteBuffer.allocate(length+4);
        b.put(length);
        b.put(type.code);
        //Due to Little endian structure on BLE Mini
        b.put((byte)(destination & 0xFF));
        b.put((byte)(destination >> 8));

        b.put(targetApplication.getId());
        b.put(message);

        // Write the data msg to the
        rx.setValue(b.array());
        btService.writeCharacteristic(rx);
    }

    @Override
    public void sendBroadcast(MeshNodeApplication targetApplication, byte[] message) {
        if(!connection.isActive()) {
            return;
        }

        // Special case for broadcast since it doesn't include a destination
        BluetoothGattCharacteristic rx = btService.getSupportedGattService().
                getCharacteristic(MeshGattAttributes.UUID_MESH_RX);
        // Length = length(2) + type(1) + destination(2) + applicationId(2) + message.length
        byte length = (byte) ( 1+ message.length);

        // Build the data msg to be sent
        ByteBuffer b = ByteBuffer.allocate(length+2);
        b.put(length);
        b.put(MessageType.BROADCAST.code);
        b.put(targetApplication.getId());
        b.put(message);

        // Write the data msg to the
        rx.setValue(b.array());
        btService.writeCharacteristic(rx);
    }

    @Override
    public void sendBroadcast(MeshNodeApplication targetApplication, String message) {
        sendBroadcast(targetApplication, message.getBytes());

    }

    @Override
    public void sendGroupBroadcast(short destination, MeshNodeApplication targetApplication, byte[] message) {
        sendMessageHelper(MessageType.GROUP_BROADCAST, destination, targetApplication, message);
    }

    @Override
    public void sendGroupBroadcast(short destination, MeshNodeApplication targetApplication, String message) {
        sendGroupBroadcast(destination, targetApplication, message.getBytes());
    }

    @Override
    public void sendStatelessMessage(short destination, MeshNodeApplication targetApplication, byte[] message) {
        sendMessageHelper(MessageType.STATELESS_MESSAGE, destination, targetApplication, message);
    }

    @Override
    public void sendStatelessMessage(short destination, MeshNodeApplication targetApplication, String message) {
        sendStatelessMessage(destination, targetApplication, message.getBytes());
    }

    @Override
    public void sendStatefulMessage(short destination, MeshNodeApplication targetApplication, byte[] message) {
        sendMessageHelper(MessageType.STATEFUL_MESSAGE, destination, targetApplication, message);
    }

    @Override
    public void sendStatefulMessage(short destination, MeshNodeApplication targetApplication, String message) {
        sendStatefulMessage(destination, targetApplication, message.getBytes());
    }

    @Override
    public void registerCallback(MeshMessageCallback callback, MeshNodeApplication targetApplication) {
        if(callback == null || targetApplication == null) {
            throw new IllegalArgumentException("Callback nor targetApplication may not be null");
        }

        Set<MeshMessageCallback> callBackSet = callbacks.get(targetApplication.getId());

        if(callBackSet == null) {
            // Create a new list for the target application
            callBackSet = new HashSet<>();
        }

        // Add to the set, guaranteeing only one instance of the same callback
        callBackSet.add(callback);

    }

    @Override
    public void deregisterCallback(MeshMessageCallback callback, MeshNodeApplication targetApplication) {
        if(callback == null || targetApplication == null) {
            throw new IllegalArgumentException("Callback nor targetApplication may not be null");
        }

        Set<MeshMessageCallback> callBackSet = callbacks.get(targetApplication.getId());
        if(callBackSet != null) {
            callBackSet.remove(callback);
        }
    }

    @Override
    public void deregisterCallback(MeshMessageCallback callback) {
        for(MeshNodeApplication app : callbacks.keySet()) {
            deregisterCallback(callback, app);
        }
     }

    private MeshCharacteristicUpdatedCallback characteristicUpdatedCallback = new MeshCharacteristicUpdatedCallback() {
        @Override
        public void CharacteristicUpdated(byte [] data) {
            // Conversion from little endian to big endian (which Java uses)
            short source = (short) ((data[0] & 0xFF) | (data[1] << 8));
            byte appId = data[2];
            byte[] message = new byte[data.length - 3];
            // Copy the pure message
            System.arraycopy(data, 3, message, 0, data.length - 3);

            MeshNodeApplication app = getApplicationFromByte(appId);

            if(app == null) {
                // this app isn't int in the map
                return;
            }

            Set<MeshMessageCallback> callBackSet = callbacks.get(app);
            if(callBackSet == null) {
                // No callback objects listed for this app
                return;
            }

            for(MeshMessageCallback callback : callBackSet) {
                callback.onMessageReceived(source, null, null);
            }

        }
    };

    private MeshNodeApplication getApplicationFromByte(byte id) {
        for(MeshNodeApplication app : callbacks.keySet()) {
            if(app.getId() == id) {
                return app;
            }
        }

        return null;
    }
}
