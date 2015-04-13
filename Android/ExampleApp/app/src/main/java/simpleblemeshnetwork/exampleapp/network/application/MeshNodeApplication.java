package simpleblemeshnetwork.exampleapp.network.application;

import java.util.List;

/**
 * @author Sebastian Blomberg
 */
public interface MeshNodeApplication {
    /**
     * Returns a <strong>unique</strong> identifier for this application
     * @return ID
     */
    public byte getId();

    /**
     * Returns the name of this application
     * @return name
     */
    public String getName();

    /**
     * Returns a textual description of this application.
     * Should include the purpose.
     * @return description
     */
    public String getDescription();

    /**
     * Returns the available operation codes (OP-Codes) of this application
     * @return
     */
    public ApplicationOperationCode[] getOperationCodes();

    /**
     * Constructs an application message with specified OP-Code to be processed
     * by this application on a node.
     * @param opCode
     * @param data
     * @return constructed message
     */
    public byte[] constructMessage(ApplicationOperationCode opCode, byte[] data);
}
