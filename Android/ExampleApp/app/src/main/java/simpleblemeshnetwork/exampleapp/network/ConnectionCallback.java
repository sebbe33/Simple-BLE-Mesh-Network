package simpleblemeshnetwork.exampleapp.network;

/**
 * @author Sebastian Blomberg
 */
public interface ConnectionCallback {
    public void onSuccess(Connection connection);

    public void onFailure(String message);
}
