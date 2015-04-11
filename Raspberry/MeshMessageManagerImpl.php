<?php
require_once 'MeshMessageManager.php';
require_once 'MeshNodeApplication.php';
require_once 'MeshMessageType.php';

class MeshMessageManagerImpl implements MeshMessageManager{
    const DEFAULT_BAUD_RATE = 57600;
    const DEFAULT_DEVICE = "/dev/ttyAMA0";
    
    private $serial;
   
    public function __construct(PhpSerial $serial) {
        if(!($serial instanceof PhpSerial)) {
            throw new InvalidArgumentException("The serial interface must be"
                    . " an instance of PhpSerial");
        }

        $this->serial = $serial;
        $serial->deviceSet(MeshMessageManagerImpl::DEFAULT_DEVICE);
        $serial->confBaudRate(MeshMessageManagerImpl::DEFAULT_BAUD_RATE);
        $serial->confParity("none");
        $serial->confCharacterLength(8);
        $serial->confStopBits(1);
        $serial->deviceOpen();
    }

    public function sendBroadcast(MeshNodeApplication $targetApplication, $message) {
        $this->sendMessageHelper(false, MeshMessageType::BROADCAST, 
                $targetApplication,  $message);
    }
    
    public function sendGroupBroadcast($destination, 
            MeshNodeApplication $targetApplication, $message) {
        $this->sendMessageHelper($destination, MeshMessageType::GROUP_BROADCAST, 
                $targetApplication,  $message);
    }

    public function sendStatelessMessage($destination, 
            MeshNodeApplication $targetApplication, $message) {
        $this->sendMessageHelper($destination, MeshMessageType::STATELESS_MESSAGE,
                $targetApplication, $message);
    }

    public function sendStatefulMessage($destination, 
            MeshNodeApplication $targetApplication, $message) {
        $this->sendMessageHelper($destination, MeshMessageType::STATEFUL_MESSAGE, 
                $targetApplication, $message);
    }
    
    private function sendMessageHelper($destination, $type, 
            MeshNodeApplication $targetApplication, $message) {
        if(!is_numeric($destination)) {
            throw new InvalidArgumentException("Destination must be numeric");
        }
        
        $data = array(
            0xE6, // indentifier
            (3 + strlen($message)) & 0xFF, // Data length (including header) : 1 byte
            (strlen($message) + 1) & 0xFF, // Message length (including app type) : 1 byte
            $type & 0xFF, // Type 1 byte
                );
        
        if($destination !== false) {
            $data[] = ($destination) & 0xFF;
            $data[] = ($destination>>8) & 0xFF;                    
            // Increase message length by 2
            $data[1] += 2;
        }
        
        $data[] = $targetApplication->getId() & 0xFF;
        $dataAsString = $this->byteArrayToString($data);
        $this->serial->sendMessage($dataAsString.$message);
    }
    
    public function byteArrayToString($byteArray) {
        return implode(array_map("chr", $byteArray));
    }
    
    public function __destruct() {
       $this->serial->deviceClose();
   }
}
