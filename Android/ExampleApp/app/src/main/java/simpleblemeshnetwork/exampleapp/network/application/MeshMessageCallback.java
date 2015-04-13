package simpleblemeshnetwork.exampleapp.network.application;

/**
 * @author Markus Andersson, Sebastian Blomber
 */
public interface MeshMessageCallback {
    public void onMessageReceived(short source, MeshNodeApplication app, byte[] message);
}
