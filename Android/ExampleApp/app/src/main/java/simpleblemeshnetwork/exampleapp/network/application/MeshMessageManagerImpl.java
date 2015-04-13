package simpleblemeshnetwork.exampleapp.network.application;

import android.bluetooth.BluetoothGattCharacteristic;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

import java.nio.ByteBuffer;

import simpleblemeshnetwork.exampleapp.network.MeshGattAttributes;
import simpleblemeshnetwork.exampleapp.network.connection.Connection;
import simpleblemeshnetwork.exampleapp.network.connection.MeshBluetoothService;
import simpleblemeshnetwork.exampleapp.network.connection.MeshCharacteristicUpdatedCallback;

/**
 * @author Sebastian Blomberg
 */
public class MeshMessageManagerImpl implements MeshMessageManager {
    private MeshBluetoothService btService;
    private Connection connection;
    MeshMessageCallback meshMessageCallback;


    public MeshMessageManagerImpl(MeshBluetoothService btService, Connection connection) {
        this.btService = btService;
        this.connection = connection;

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
    public boolean registerCallback(MeshMessageCallback callback) {
        if(callback != null){
            meshMessageCallback = callback;
            return true;
        }
        return false;
    }

    @Override
    public boolean unregisterCallback(MeshMessageCallback callback) {
        return false;
    }

    private MeshCharacteristicUpdatedCallback characteristicUpdatedCallback = new MeshCharacteristicUpdatedCallback() {
        @Override
        public void CharacteristicUpdated(byte [] data) {

                Log.d("TXDATA:", "" + data[0] + data[1]);
                //TODO: first byte : app
                //second byte:
        }
    };

}
