package simpleblemeshnetwork.exampleapp.network.connection;

import android.bluetooth.BluetoothGattCharacteristic;

/**
 * Created by Markus on 2015-04-13.
 */
public interface MeshCharacteristicUpdatedCallback {

    public void CharacteristicUpdated(byte [] data);
}
