package simpleblemeshnetwork.exampleapp.network;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * @author Sebastian Blomberg
 * @author Markus Andersson
 */
public class ConnectionManagerImpl implements ConnectionManager {
    public static final byte MESH_IDENTIFIER = (byte) 0xBC;

    private final BluetoothAdapter bluetoothAdapter;
    private final Context context;
    private MeshBluetoothService meshBluetoothService;
    private ConnectionImpl activeConnection;
    private ConnectionCallback connectionCallback;
    private AvailableNetworkCallback scanResultCallback;
    private Thread scanThread;


    public ConnectionManagerImpl(BluetoothAdapter bluetoothAdapter, Context context) {
        this.bluetoothAdapter = bluetoothAdapter;
        this.context = context;
        initializeServiceConnection();
        startBroadcastReceiver();
    }

    @Override
    public void searchForAvailableNetworks(AvailableNetworkCallback callback) {
        searchForAvailableNetworks(callback, ConnectionManager.DEFAULT_SEARCH_PERIOD);
    }

    @Override
    public void searchForAvailableNetworks(AvailableNetworkCallback callback, int searchPeriod) {
        scanThread = new Thread(new BluetoothScanThread(searchPeriod, callback));
        scanThread.start();
        scanResultCallback = callback;
    }

    @Override
    public void establishConnection(NetworkIdentifier networkId,
                                          ConnectionCallback callback) {
        if(networkId.getClass() != NetworkIdentifierImpl.class) {
            // Not from this manager
            return;
        }

        if(activeConnection != null) {
            discontinueConnection(activeConnection);
        }

        connectionCallback = callback;
        activeConnection = new ConnectionImpl(true, networkId);
        Log.d("HEJ", meshBluetoothService == null? "null" : "ins");
        meshBluetoothService.connect(((NetworkIdentifierImpl) networkId).getConnectionDetails().getAddress());
    }

    @Override
    public void discontinueConnection(Connection connection) {
        meshBluetoothService.disconnect();
        activeConnection.setActive(false);
        activeConnection = null;
    }


    private void initializeServiceConnection() {
        Intent gattServiceIntent = new Intent(context, MeshBluetoothService.class);
        context.bindService(gattServiceIntent, mServiceConnection, context.BIND_AUTO_CREATE);
        Log.d("BT", "Binding service");
    }

    private void startBroadcastReceiver() {
        IntentFilter mGattUpdateIntentFilter = new IntentFilter();
        mGattUpdateIntentFilter.addAction(MeshBluetoothService.ACTION_GATT_CONNECTED);
        mGattUpdateIntentFilter.addAction(MeshBluetoothService.ACTION_GATT_SERVICES_DISCOVERED);
        context.registerReceiver(broadcastReceiver, mGattUpdateIntentFilter);
    }

    private void stopBroadcastReceiver() {
        context.unregisterReceiver(broadcastReceiver);
    }

    private class BluetoothScanThread implements Runnable {
        private int scanTimeout;
        private AvailableNetworkCallback callback;
        private ScanResultCallback bleCallback = new ScanResultCallback();

        public BluetoothScanThread(int scanTimeout, AvailableNetworkCallback callback) {
            this.scanTimeout = scanTimeout;
            this.callback = callback;
        }


        @Override
        public void run() {
            synchronized (this) {
                Log.d("BT", "Starting scan");
                bluetoothAdapter.startLeScan(bleCallback);


                try {
                    this.wait(scanTimeout);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                Log.d("debug", "woke up");
                bluetoothAdapter.stopLeScan(bleCallback);

                if (scanResultCallback != null) {
                    scanResultCallback.onSearchDone(bleCallback.getResults());
                }

            }
        }
    };

    private class ScanResultCallback implements BluetoothAdapter.LeScanCallback {
        private List<NetworkIdentifierImpl> results = new ArrayList<>();

        public List<NetworkIdentifierImpl> getResults() {
            return results;
        }

        @Override
        public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {
            // If the advertising data isn't from a valid mesh network
            if(device.getName() == null || scanRecord[3] != 3 || scanRecord[4] != MESH_IDENTIFIER) {
                return;
            }

            short identifier = (short) ((scanRecord[6] << 8) | scanRecord[5]);
            Log.d("BTRESULT", identifier + " ");
            NetworkIdentifierImpl id = new NetworkIdentifierImpl(identifier, device.getName());

            if(!results.contains(id)) {
                id.setConnectionDetails(id.new ConnectionDetails(device.getAddress(), rssi));
                results.add(id);
                if(scanResultCallback != null) {
                    scanResultCallback.onNetworkFound(id);
                }
            } else {
                id = results.get(results.indexOf(id));
                if(id.getConnectionDetails().getRssi() < rssi) {
                    id.setConnectionDetails(id.new ConnectionDetails(device.getAddress(), rssi));
                }
            }
        }
    }

    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if(MeshBluetoothService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)){
                if(connectionCallback != null) {
                    connectionCallback.onSuccess(activeConnection);
                    connectionCallback = null;
                }
            }
        }
    };

    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            meshBluetoothService = ((MeshBluetoothService.LocalBinder) service).getService();
            Log.d("BT", "Initiated RBLService");
            if (!meshBluetoothService.initialize()) {
                Log.e("TAG", "Unable to initialize Bluetooth");
                // TODO :: THROW EXCEPTION
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            meshBluetoothService = null;
        }
    };

    public void onPause(){
        if(scanThread.getState() == Thread.State.TIMED_WAITING) {
            Log.d("debug","is on scan");
            scanResultCallback = null;
            synchronized (scanThread){
                scanThread.notify();
            }
        }

        Log.d("Thread",scanThread.getState().toString() +"");
        context.stopService(new Intent(context, MeshBluetoothService.class));
        context.unbindService(mServiceConnection);
        stopBroadcastReceiver();
    }

    public void onResume () {
        initializeServiceConnection();
        startBroadcastReceiver();
    }

    public MeshBluetoothService getbtService(){
        return meshBluetoothService;
    }
}