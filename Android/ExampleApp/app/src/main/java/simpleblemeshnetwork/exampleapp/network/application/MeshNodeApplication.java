package simpleblemeshnetwork.exampleapp.network.application;

import java.util.List;

/**
 * @author Sebastian Blomberg
 */
public interface MeshNodeApplication {
    public byte getId();
    public String getName();
    public String getDescription();
    public ApplicationOperationCode[] getOperationCodes();
    public byte[] constructMessage(ApplicationOperationCode opCode, byte[] data);
}
