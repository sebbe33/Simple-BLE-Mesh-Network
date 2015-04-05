package simpleblemeshnetwork.exampleapp;

import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;

import java.util.List;

import simpleblemeshnetwork.exampleapp.network.AvailableNetworkCallback;
import simpleblemeshnetwork.exampleapp.network.Connection;
import simpleblemeshnetwork.exampleapp.network.ConnectionCallback;
import simpleblemeshnetwork.exampleapp.network.ConnectionManager;
import simpleblemeshnetwork.exampleapp.network.ConnectionManagerImpl;
import simpleblemeshnetwork.exampleapp.network.MeshNodeApplication;
import simpleblemeshnetwork.exampleapp.network.MeshNodeApplicationImpl;
import simpleblemeshnetwork.exampleapp.network.MessageType;
import simpleblemeshnetwork.exampleapp.network.NetworkIdentifier;
import simpleblemeshnetwork.exampleapp.network.NetworkIdentifierImpl;


public class MainActivity extends ActionBarActivity {
    private ArrayAdapter<String> applicationTypeArrayAdapter;
    private ArrayAdapter<NetworkIdentifier> scanResultsArrayAdapter;
    private Spinner appTypeSpinner, messageTypeSpinner;
    private EditText messageTextField;
    private Button sendButton, disconnectButton;

    private ConnectionManager conMan;
    private AlertDialog alert;
    private Button scanAgainButton;
    private Connection activeConnection;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        String[] appTypeValues = new String[MeshNodeApplicationImpl.values().length];
        int i =0;
        for(MeshNodeApplication app : MeshNodeApplicationImpl.values()) {
            appTypeValues[i++] = app.getName();
        }

        disconnectButton = (Button) findViewById(R.id.disconnectButton);
        disconnectButton.setOnClickListener(disconnectButtonListener);

        applicationTypeArrayAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, appTypeValues);
        appTypeSpinner = (Spinner) findViewById(R.id.applicationTypeSpinner);
        appTypeSpinner.setAdapter(applicationTypeArrayAdapter);

        messageTypeSpinner = (Spinner) findViewById(R.id.messageTypeSpinner);
        messageTextField = (EditText) findViewById(R.id.messageTextField);

        sendButton = (Button) findViewById(R.id.sendButton);
        sendButton.setOnClickListener(sendButtonListener);

        disconnectButton = (Button) findViewById(R.id.disconnectButton);

        // Initiate the connection manager
        BluetoothManager mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = mBluetoothManager.getAdapter();
        conMan = new ConnectionManagerImpl(adapter, this);
        if (!adapter.isEnabled()) {
            Intent enableBtIntent = new Intent(
                    BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, 1);
            Log.d("BT", "Enabled");
        }

        scanResultsArrayAdapter = new ArrayAdapter<NetworkIdentifier>(this, android.R.layout.select_dialog_item);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Scanning...");
        builder.setNeutralButton("Scan again", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
            }
        });
        builder.setAdapter(scanResultsArrayAdapter, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int item) {
                conMan.establishConnection(scanResultsArrayAdapter.getItem(item), new ConnectionCallback() {
                    @Override
                    public void onSuccess(Connection connection) {
                        activeConnection = connection;
                    }

                    @Override
                    public void onFailure(String message) {

                    }
                });
                scanResultsArrayAdapter.clear();
            }
        });
        alert = builder.create();
        alert.show();
        alert.setCanceledOnTouchOutside(false);
        scanAgainButton = alert.getButton(DialogInterface.BUTTON_NEUTRAL);
        scanAgainButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                conMan.searchForAvailableNetworks(scanResultCallback);
                scanAgainButton.setEnabled(false);
                alert.setTitle("Scanning...");
            }
        });

        conMan.searchForAvailableNetworks(scanResultCallback);
        scanAgainButton.setEnabled(false);
        alert.setTitle("Scanning...");
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onResume() {
        super.onResume();
        conMan.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
        conMan.onPause();
    }

    private View.OnClickListener sendButtonListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            byte app = MeshNodeApplicationImpl.values()[appTypeSpinner.getSelectedItemPosition()]
                            .getId();
            String msg = messageTextField.getText().toString();

            switch (MessageType.valueOf(messageTypeSpinner.getSelectedItem().toString())) {
                case BROADCAST:
                    break;
                case GROUP_BROADCAST:
                    break;
                case STATELESS_MESSAGE:
                    break;
                case STATEFUL_MESSAGE:
                    break;
            }

        }
    };

    private View.OnClickListener disconnectButtonListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            conMan.discontinueConnection(activeConnection);
            alert.show();
            conMan.searchForAvailableNetworks(scanResultCallback);
            scanAgainButton.setEnabled(false);
            alert.setTitle("Scanning...");
        }
    };

    private AvailableNetworkCallback scanResultCallback = new AvailableNetworkCallback() {
        @Override
        public void onNetworkFound(final NetworkIdentifier result) {
            runOnUiThread(new Runnable() {
                public void run() {
                    scanResultsArrayAdapter.add(result);
                    scanResultsArrayAdapter.notifyDataSetChanged();
                }
            });

        }

        @Override
        public void onSearchDone(final List<? extends NetworkIdentifier> results) {
            runOnUiThread(new Runnable() {
                public void run() {
                    if(results.size() == 0) {
                        alert.setTitle("No mesh networks found");
                    } else {
                        alert.setTitle("Tap to connect");
                    }
                    scanAgainButton.setEnabled(true);
                }
            });
        }
    };
}
