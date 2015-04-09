package simpleblemeshnetwork.exampleapp.network.application;

import android.bluetooth.BluetoothGattCharacteristic;

import java.nio.ByteBuffer;

import simpleblemeshnetwork.exampleapp.network.MeshGattAttributes;
import simpleblemeshnetwork.exampleapp.network.connection.Connection;
import simpleblemeshnetwork.exampleapp.network.connection.MeshBluetoothService;

/**
 * @author Sebastian Blomberg
 */
public class MeshMessageManagerImpl implements MeshMessageManager {
    private MeshBluetoothService btService;
    private Connection connection;

    public MeshMessageManagerImpl(MeshBluetoothService btService, Connection connection) {
        this.btService = btService;
        this.connection = connection;
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
}
