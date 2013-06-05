<?php
// $Id$
function transactionreport_all()
{
    //$transreport_content = "<TABLE><TR><TH>Team Name</TH><TH>Dropped Player</TH><TH>Added Player</TH></TR>";
    $header[] = array('data'=>t('Team Name'));
    $header[] = array('data'=>t('Dropped Player'));
    $header[] = array('data'=>t('Added Player'));
    
    $currentteam = "";
    $currentyear = "";
    $currentweek = "99";

	$gameweek = getLeagueWeek();

    $leaguedb = connectdbs();
    
    $leagueurl = $_SERVER['SERVER_NAME'];
    if (substr($leagueurl,0,4) == "www.")
    {
        $leagueurl = substr($leagueurl,4);
    }
    if ($leagueurl == "open-stats.com")
    {
        $leagueurl = "ncaaproject.com";
    }
	$leagueidquery = "SELECT leagueid FROM leagues WHERE leagueurl = '$leagueurl'";
	$leagueid = $leaguedb->queryOne($leagueidquery);
	DBerror($leagueidquery, $leagueid);

    $DropPlayerQuery = "SELECT transtime, dropplayerid, addplayerid, toir, fromir, teams.teamfirstname as transteam FROM transactions, teams WHERE transactions.teamid = teams.teamid AND transactions.leagueid = $leagueid ORDER BY transtime DESC, transactions.teamid, transactions.transid";
    $DropPlayerResult = $leaguedb->query($DropPlayerQuery );

   	while ($playerrow = $DropPlayerResult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
		extract ($playerrow);
        
        $playerweek = date(W,strtotime($transtime))-35;
        if ($currentweek != $playerweek)
        {
            $currentweek = $playerweek;
            $row[] = array('data'=>"Week $currentweek", 'colspan'=>3, 'style'=>'text-align: center; font-weight: bold');
//            $row[] = array('data'=>"Week $currentweek", 'colspan'=>3);
            $rows[] = $row;
            $row = array();
        }

        $DropPlayerFullName = getPlayerName($dropplayerid, $leaguedb);
        if ($DropPlayerFullName == '')
        {
            $DropPlayerFullName = getNFLPlayerName($dropplayerid, $leaguedb);
        }
        
        //        $DropPlayerFullName = $DropPlayerrow['firstname'].' '.$DropPlayerrow['lastname'].' - '.$DropPlayerrow['pos'].' - '.$DropPlayerrow['lgteamabbr'];
        if ($toir)
        {
            $DropPlayerFullName .= " (to IR)";
        }

        $AddPlayerFullName = getPlayerName($addplayerid, $leaguedb);
        if ($AddPlayerFullName == '')
        {
            $AddPlayerFullName = getNFLPlayerName($addplayerid, $leaguedb);
        }
        
        if ($fromir)
        {
            $AddPlayerFullName .= " (from IR)";
        }
        $row[] = array('data'=>$transteam);
        $row[] = array('data'=>$DropPlayerFullName);
        $row[] = array('data'=>$AddPlayerFullName);
        $rows[] = $row;
        $row = array();
    }
//    $leaguedb->disconnect();
//    $blocks['content'] = $transreport_content."</TABLE>";
//    return $blocks;
    $transreport_content = theme('table', $header, $rows);
    return $transreport_content;
}
?>