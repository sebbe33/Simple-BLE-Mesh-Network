package simpleblemeshnetwork.exampleapp.network.application;

/**
 * @author Sebastian Blomberg
 */
public interface MeshMessageManager {
    public void sendBroadcast(MeshNodeApplication targetApplication, byte[] message);

    public void sendBroadcast(MeshNodeApplication targetApplication, String message);

    public void sendGroupBroadcast(short destination, MeshNodeApplication targetApplication,
                                   byte[] message);

    public void sendGroupBroadcast(short destination, MeshNodeApplication targetApplication,
                                   String message);

    public void sendStatelessMessage(short destination, MeshNodeApplication targetApplication,
                                   byte[] message);

    public void sendStatelessMessage(short destination, MeshNodeApplication targetApplication,
                                   String message);

    public void sendStatefulMessage(short destination, MeshNodeApplication targetApplication,
                                     byte[] message);

    public void sendStatefulMessage(short destination, MeshNodeApplication targetApplication,
                                     String message);
}

