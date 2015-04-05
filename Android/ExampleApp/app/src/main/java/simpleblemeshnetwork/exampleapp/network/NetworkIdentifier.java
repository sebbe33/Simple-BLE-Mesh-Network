package simpleblemeshnetwork.exampleapp.network;

/**
 * @author Sebastian Blomberg
 */
public interface NetworkIdentifier {
    /**
     * Returns the identification (ID) of a network.
     * <strong>Globally unique</strong>
     * @return ID
     */
    public short getId();

    /**
     * Returns the network name
     * @return network name
     */
    public String getName();

}
