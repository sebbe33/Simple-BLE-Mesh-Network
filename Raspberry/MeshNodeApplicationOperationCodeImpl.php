<?php

require_once 'MeshNodeApplicationOperationCode.php';

class MeshNodeApplicationOperationCodeImpl {
    private $code;
    private $description;
    
    public function __construct($code, $description) {
        $this->code = $code;
        $this->description = $description;
    }


    public function getCode() {
        return $this->code;
    }
    
    public function getDescription() {
        return $this->description;
    }
}
