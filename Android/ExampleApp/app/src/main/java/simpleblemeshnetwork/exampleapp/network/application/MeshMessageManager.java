package simpleblemeshnetwork.exampleapp.network.application;

/**
 * @author Sebastian Blomberg
 */
public interface MeshMessageManager {
    /**
     * Sends a broadcast message with specified application and raw byte-message
     * @param targetApplication The application which will receive the message
     * @param message The message
     */
    public void sendBroadcast(MeshNodeApplication targetApplication, byte[] message);

    /**
     * Sends a broadcast message with specified application and message
     * @param targetApplication The application which will receive the message
     * @param message The message
     */
    public void sendBroadcast(MeshNodeApplication targetApplication, String message);

    /**
     * Sends a group broadcast to be received by the the specified group of nodes and processed
     * by their respective targeted application
     * @param destination The ID of the receiving group
     * @param targetApplication The application which will receive the message
     * @param message The message
     */
    public void sendGroupBroadcast(short destination, MeshNodeApplication targetApplication,
                                   byte[] message);

    /**
     * Sends a group broadcast to be received by the the specified group of nodes and processed
     * by their respective targeted application
     * @param destination The ID of the receiving group
     * @param targetApplication The application which will receive the message
     * @param message The message
     */
    public void sendGroupBroadcast(short destination, MeshNodeApplication targetApplication,
                                   String message);

    /**
     * Sends a stateless the be received by the specified node and processed by its specified
     * application.
     * @param destination The ID of the node to receive the message
     * @param targetApplication The application which will receive the message
     * @param message The message
     */
    public void sendStatelessMessage(short destination, MeshNodeApplication targetApplication,
                                   byte[] message);

    /**
     * Sends a stateless the be received by the specified node and processed by its specified
     * application.
     * @param destination The ID of the node to receive the message
     * @param targetApplication The application which will receive the message
     * @param message The message
     */
    public void sendStatelessMessage(short destination, MeshNodeApplication targetApplication,
                                   String message);

    /**
     * Sends a stateful the be received by the specified node and processed by its specified
     * application.
     * @param destination The ID of the node to receive the message
     * @param targetApplication The application which will receive the message
     * @param message The message
     */
    public void sendStatefulMessage(short destination, MeshNodeApplication targetApplication,
                                     byte[] message);
    /**
     * Sends a stateful the be received by the specified node and processed by its specified
     * application.
     * @param destination The ID of the node to receive the message
     * @param targetApplication The application which will receive the message
     * @param message The message
     */
    public void sendStatefulMessage(short destination, MeshNodeApplication targetApplication,
                                     String message);

    /**
     * Registers a callback object which is called upon when an incoming message for the specified
     * application is processed.
     * @param callback The object which will be called upon
     * @param targetApplication The application for which the callback will take place
     */
    public void registerCallback(MeshMessageCallback callback, MeshNodeApplication targetApplication);

    /**
     * Deregisters a specified callback object for the specific application.
     * @param callback The callback to be removed
     * @param targetApplication The application for which the callback is registered on
     */
    public void deregisterCallback(MeshMessageCallback callback, MeshNodeApplication targetApplication);

    /**
     * Deregisters a specified callback object for all applications.
     * @param callback The callback to be removed
     */
    public void deregisterCallback(MeshMessageCallback callback);
}

