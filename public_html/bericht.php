<?php
if(isset($_POST['boodschap1'])) {
    $_POST['boodschap'] = $_POST['boodschap1'];
    //if(isset($_POST['boodschap2'])) $_POST['boodschap'] .= $_POST['boodschap2'];
    
    if(strlen($_POST['boodschap']) > 64) $_POST['boodschap'] = substr($_POST['boodschap'], 0, 64);
    require "database.php";
    $database = Database::getInstance();
    $database->prepare("INSERT INTO berichten (messenge) VALUES (:boodschap)");
    $database->bindParam(":boodschap", $_POST['boodschap']);
    $database->execute();
    header('Location: index.php');
}