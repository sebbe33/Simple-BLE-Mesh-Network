<?php

require_once 'MeshNodeApplication.php';

interface MeshMessageManager
{
    public function sendBroadcast(MeshNodeApplication $targetApplication, $message);
    
    public function sendGroupBroadcast($destination, 
            MeshNodeApplication $targetApplication, $message);

    public function sendStatelessMessage($destination, 
            MeshNodeApplication $targetApplication, $message);

    public function sendStatefulMessage($destination, 
            MeshNodeApplication $targetApplication, $message);
}
