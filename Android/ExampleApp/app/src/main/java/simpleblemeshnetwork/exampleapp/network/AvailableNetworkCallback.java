package simpleblemeshnetwork.exampleapp.network;


import java.util.List;

/**
 * @author Sebastian Blomberg
 */
public interface AvailableNetworkCallback {
    public void onNetworkFound(NetworkIdentifier result);
    public void onSearchDone(List<? extends NetworkIdentifier> results);
}
