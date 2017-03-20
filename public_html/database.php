<?php



/**
 * Loads the database object
 *
 * @author Dennis Ruigrok
 */



class Database {
    private static $instance;
    
    protected $pdo;
    private $statement;
    
    public static function getInstance()
    {
        if (!isset(static::$instance)) {
            static::$instance = new static;
        }
        return static::$instance;
    }
    protected function __clone() {
        
    }
    protected function __construct() {             
        
        $hostname = "127.0.0.1";
        $databaseName = "erwin50";
        $login = "erwin50";
        $password = "fill this out";
        $dsn = "mysql:host=$hostname;dbname=$databaseName";

        $options = array(
            PDO::MYSQL_ATTR_INIT_COMMAND => 'SET NAMES utf8',
        ); 

        try {
            $this->pdo = new PDO($dsn, $login, $password, $options);
        } catch(Exception $e) {
            die ("database connection failed");
        };
        
    }
    
    function prepare($sql) {
        $this->statement = $this->pdo->prepare($sql);
        //$b=$a->prepare("UPDATE `users` SET user=:var");

    }
    
    function bindParam($param, $value) {
        
        $this->statement->bindParam($param, $value);
    }
    
    function limit($limit) {
        $this->statement->bindParam(':limit', $limit, PDO::PARAM_INT);

    }
    
    function execute() {
        $this->statement->execute();
    }
    
    function getAll() {
        return $this->statement->fetchAll();
    }
    
    function getPDOObject() {
        return $this->pdo;
    }
    function getStatement() {
        return $this->statement;
    }
    
    
}
function lastid($database) {
    $database->prepare("SELECT LAST_INSERT_ID() AS lastid");
    $database->execute();
    $lastid = $database->getAll()[0]['lastid'];
    return $lastid;
 }

