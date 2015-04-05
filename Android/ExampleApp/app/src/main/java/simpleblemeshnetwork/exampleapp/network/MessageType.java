package simpleblemeshnetwork.exampleapp.network;

/**
 * @author Sebastian Blomberg
 */
public enum MessageType {
    BROADCAST((byte) 0x00),
    GROUP_BROADCAST((byte) 0x01),
    STATELESS_MESSAGE((byte) 0x02),
    STATEFUL_MESSAGE((byte) 0x03);

    public final byte code;

    private MessageType(byte code) {
        this.code =code;
    }
}
