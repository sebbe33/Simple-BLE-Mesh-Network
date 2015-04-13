<?php

require_once 'MeshNodeApplicationImpl.php';

class MeshNodeApplicationImpl implements MeshNodeApplication {
    private $id, $name, $opCodes;
    
    public function __construct($id, $name, $opCodes) {
        $this->id = $id;
        $this->name = $name;
        $this->opCodes = $opCodes;

    }
    
    public function getId() {
        return $this->id;
    }
    
    public function getName() {
        return $this->name;
    }

    public function getOperationCode($code) {
        if(array_key_exists($code, $this->opCodes)) {
            return $this->opCodes[$code];
        }
        
        return false;
    }
    
    public function getOperationCodes() {
        return $this->opCodes;
    }
    
    public function constructMessage($OPCode, $message) {
        $safeOPCode = false;
        if($OPCode instanceof MeshNodeApplicationOperationCode) {
            $safeOPCode = $this->getOperationCode($OPCode->getCode());  
        } else {
            $safeOPCode = $this->getOperationCode($OPCode);
        }
        
        if(!$safeOPCode) {
            // Invalid OP code
            return;
        }
        
        return chr($safeOPCode->getCode()).$message;
    }
}
