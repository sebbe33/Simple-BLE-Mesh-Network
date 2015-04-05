package simpleblemeshnetwork.exampleapp.network;
/**
 * @author Sebastian Blomberg
 */
public class ConnectionImpl implements Connection {
    private boolean isActive;
    private NetworkIdentifier networkIdentifier;

    public ConnectionImpl(boolean isActive, NetworkIdentifier networkIdentifier) {
        this.isActive = isActive;
        this.networkIdentifier = networkIdentifier;
    }

    @Override
    public boolean isActive() {
        return isActive;
    }

    @Override
    public NetworkIdentifier getNetworkIdentifier() {
        return networkIdentifier;
    }

    public void setActive(boolean isActive) {
        this.isActive = isActive;
    }

}
