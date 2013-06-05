<?php
function connectdbs()
{
    require_once 'MDB2.php';
    static $leaguedb;
    // Connect to CFFL and CTGFFL database.
    //
    $user = 'dlvadmin';
    $pass = 't15Tebow';
    $dbname1='ncaap';
    //$dbname2='ctgffld_xoopsdb';
    $host='localhost';
    $dsn1 = "mysql://$user:$pass@$host/$dbname1";
    $leaguedb = MDB2::connect($dsn1, false);
    //echo "TEST";
    if (MDB2::isError($leaguedb)) {

        echo 'Standard Message: ' . $leaguedb->getMessage() . "\n";
        echo 'Standard Code: ' . $leaguedb->getCode() . "\n";
        echo 'DBMS/User Message: ' . $leaguedb->getUserInfo() . "\n";
        echo 'DBMS/Debug Message: ' . $leaguedb->getDebugInfo() . "\n";

        die ($leaguedb->getMessage().' on leaguedb');
    }

    $leaguedb->setFetchMode(MDB2_FETCHMODE_ASSOC);

    /*
    $dsn2 = "mysql://$user:$pass@$host/$dbname2";
    $ctgffldb = DB::connect($dsn2, false);

    if (DB::isError($ctgffldb)) {
        echo 'Standard Message: ' . $ctgffldb->getMessage() . "\n";
        echo 'Standard Code: ' . $ctgffldb->getCode() . "\n";
        echo 'DBMS/User Message: ' . $ctgffldb->getUserInfo() . "\n";
        echo 'DBMS/Debug Message: ' . $ctgffldb->getDebugInfo() . "\n";

        die ($ctgffldb->getMessage(). ' on ctgffldb');
    }

    $ctgffldb->setFetchMode(DB_FETCHMODE_ASSOC);
    */
    return $leaguedb;
}
?>