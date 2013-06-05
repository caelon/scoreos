<?php

function admintransaction_all()
{
    $team = drupal_get_form('admintransaction_form');

    return $team;
}

function admintransaction_form($form_state)
{
    $leaguedb = connectdbs();
    if (isset($form_state['storage']['page_two']))
    {
        return admintransaction_form_page_2($form_state);
    }
//    elseif ($context['storage']['step'] == 3)
//    {
//        return starttransaction_form_page_3();
//    }
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
	
    $teamsquery = "SELECT teamid, teamfirstname as teamname FROM teams WHERE leagueid = $leagueid AND active=1 ORDER BY teamname;";
    $teamsresult = $leaguedb->query($teamsquery );

    while ($team = $teamsresult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
        $teamsarray[$team['teamid']] = $team['teamname'];
        //print_r($team);
    }

    //print_r($teamsarray);
    $form['team'] = array(
        '#type' => 'select',
        '#title' => t('Team to perform transaction on'),
        '#options' => $teamsarray,
    );
    
	$form['league'] = array(
		'#type' => 'hidden',
		'#value' => $leagueid,
	);
	
    $form['next'] = array(
        '#type' => 'submit',
        '#value' => 'Perform transaction on this team',
    );

    return $form;
}

function admintransaction_form_page_2($form_state)
{
	$leagueid = $form_state['values']['league'];
    if (!isset($leaguedb))
    {
        $leaguedb = connectdbs();
    }

	$proquery = "SELECT includepro FROM leagues WHERE leagueid = $leagueid";
	$includepro = $leaguedb->queryOne($proquery);
	DBerror($proquery, $includepro);
    
    $playersquery = "SELECT * FROM players, playerteams WHERE players.playerid = playerteams.playerid AND teamid = {$form_state['storage']['teamid']} order by lastname, firstname";
   
	$playersresult = $leaguedb->query($playersquery);
    if(PEAR::isError($playersresult)) 
    {
        var_export($form_state,TRUE);
        die('Failed to issue query, error message : ' . $playersresult->getMessage()." on ".$playersquery);
    }

	$players[0] = "Do not drop a player - add only";
   	while ($playerrow = $playersresult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
		extract ($playerrow, EXTR_PREFIX_ALL, "player");
		$teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $player_lgteamid";
		$lgteamname = $leaguedb->queryOne($teamquery);
		$playername = $player_lastname.', '.$player_firstname.' - '.$player_pos.' - '.$lgteamname;
        $players[$player_playerid] = $playername;
    }

    if ($includepro)
    {
        $nflplayersquery = "SELECT * FROM nflplayers, playerteams WHERE nflplayers.playerid = playerteams.playerid AND teamid = {$form_state['storage']['teamid']} order by lastname, firstname";
       
        $nflplayersresult = $leaguedb->query($nflplayersquery);
        if(PEAR::isError($nflplayersresult)) 
        {
            var_export($form_state,TRUE);
            die('Failed to issue query, error message : ' . $nflplayersresult->getMessage()." on ".$playersquery);
        }

        while ($nflplayerrow = $nflplayersresult->fetchrow(MDB2_FETCHMODE_ASSOC))
        {
            extract ($nflplayerrow, EXTR_PREFIX_ALL, "player");
            $teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $player_lgteamid";
            $lgteamname = $leaguedb->queryOne($teamquery);
            $playername = $player_lastname.', '.$player_firstname.' - '.$player_pos.' - '.$lgteamname;
            $players[$player_playerid] = $playername;
        }
    }
    
	$freeagentsquery = "SELECT DISTINCT playerid, firstname, lastname, lgteamid, pos, year FROM players WHERE (NOT EXISTS (SELECT playerid FROM playerteams WHERE leagueid = $leagueid AND players.playerid = playerteams.playerid) AND (pos = 'QB' OR pos = 'RB' OR pos = 'WR' OR pos = 'TE' OR pos = 'K' or pos = 'D')) ORDER BY lastname, firstname";
	$freeagentsresult = $leaguedb->query($freeagentsquery);
    if(PEAR::isError($freeagentsresult)) 
    {
        var_export($form_state,TRUE);
        die('Failed to issue query, error message : ' . $freeagentsresult->getMessage()." on ".$freeagentsquery);
    }

	$freeagents[0] = "Do not add a player - drop only";
   	while ($farow = $freeagentsresult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
		extract ($farow, EXTR_PREFIX_ALL, "fa");
		$teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $fa_lgteamid";
		$lgteamname = $leaguedb->queryOne($teamquery);
		$faname = $fa_lastname.', '.$fa_firstname.' - '.$fa_pos.' - '.$lgteamname;
        $freeagents[$fa_playerid] = $faname;
    }

    if ($includepro)
    {
        $freeagentsquery = "SELECT DISTINCT playerid, firstname, lastname, lgteamid, pos, year FROM nflplayers WHERE (NOT EXISTS (SELECT playerid FROM playerteams WHERE leagueid = $leagueid AND nflplayers.playerid = playerteams.playerid) AND (pos = 'QB' OR pos = 'RB' OR pos = 'WR' OR pos = 'TE' OR pos = 'K' or pos = 'D')) ORDER BY lastname, firstname";
        $freeagentsresult = $leaguedb->query($freeagentsquery);
        if(PEAR::isError($freeagentsresult)) 
        {
            var_export($form_state,TRUE);
            die('Failed to issue query, error message : ' . $freeagentsresult->getMessage()." on ".$freeagentsquery);
        }

        while ($farow = $freeagentsresult->fetchrow(MDB2_FETCHMODE_ASSOC))
        {
            extract ($farow, EXTR_PREFIX_ALL, "fa");
            $teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $fa_lgteamid";
            $lgteamname = $leaguedb->queryOne($teamquery);
            $faname = $fa_lastname.', '.$fa_firstname.' - '.$fa_pos.' - '.$lgteamname;
            $freeagents[$fa_playerid] = $faname;
        }
    }
    
    $form['dropplayer'] = array(
        '#type' => 'select',
        '#title' => t('Player to drop'),
        '#options' => $players,
    );

    $form['putonIR'] = array(
        '#type' => 'checkbox',
        '#title' => t('Put player on IR?'),
    );

    $form['addplayer'] = array(
        '#type' => 'select',
        '#title' => t('Player to add'),
        '#options' => $freeagents,
    );

	$form['transteam'] = array(
		'#type' => 'hidden',
		'#value' => $form_state['storage']['teamid'],
	);
	
	$form['league'] = array(
		'#type' => 'hidden',
		'#value' => $leagueid,
	);
	
    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Submit transaction',
    );

    return $form;
}

function admintransaction_form_submit($form, &$form_state)
{
	$gameweek = getLeagueWeek();

	//print_r($form_state);
    if (!isset($leaguedb))
    {
        $leaguedb = connectdbs();
    }

    $leagueurl = $_SERVER['SERVER_NAME'];
    if (substr($leagueurl,0,4) == "www.")
    {
        $leagueurl = substr($leagueurl,4);
    }
    if ($leagueurl == "open-stats.com")
    {
        $leagueurl = "ncaaproject.com";
    }
	$dvquery = "SELECT includedv FROM leagues WHERE leagueurl = '$leagueurl'";
	$includedv = $leaguedb->queryOne($dvquery);
	DBerror($dvquery, $includedv);
    
    if ($form_state['clicked_button']['#id'] == 'edit-next')
    {
        $form_state['storage']['page_two'] = TRUE;
        $form_state['storage']['teamid'] = $form_state['values']['team'];
    }
    else
    {
		extract ($form_state['values'], EXTR_PREFIX_ALL, "trans");

		$addplayername = getPlayerName($trans_addplayer, $leaguedb);
		if ($trans_dropplayer)
		{
			$dropplayername = getPlayerName($trans_dropplayer, $leaguedb);
		}
		
		$teamnamequery = "SELECT teamfirstname FROM teams WHERE teamid=$trans_transteam";
		$teamname = $leaguedb->queryOne($teamnamequery);

		if ($trans_dropplayer && !$trans_putonIR)
		{
			$transdropquery = "DELETE FROM playerteams WHERE playerid = $trans_dropplayer AND leagueid = $trans_league";
			$transdrop = $leaguedb->query($transdropquery);
			DBerror($transdropquery,$transdrop);
		}
		elseif ($trans_dropplayer && $trans_putonIR)
		{
			$transdropquery = "UPDATE playerteams SET irweek =".$gameweek['week']." WHERE playerid = $trans_dropplayer AND leagueid = $trans_league";
			$transdrop = $leaguedb->query($transdropquery);
			DBerror($transdropquery,$transdrop);		    
		}
		
		if ($trans_addplayer)
		{
			$transaddquery = "INSERT INTO playerteams VALUES($trans_addplayer, $trans_league, $trans_transteam, NULL)";
			$transadd = $leaguedb->query($transaddquery);
			DBerror($transaddquery,$transadd);
		}
        
        if ($includedv && $trans_addplayer != 0)
        {
			$dvupdatequery = "UPDATE ctgffldv SET playerid = $trans_addplayer WHERE playerid = $trans_dropplayer";
			$dvupdate = $leaguedb->query($dvupdatequery);
			DBerror($dvupdatequery,$dvupdate);
        }
        elseif ($includedv && $trans_addplayer == 0)
		{
			$dvdeletequery = "DELETE FROM ctgffldv WHERE playerid = $trans_dropplayer";
			$dvdelete = $leaguedb->query($dvdeletequery);
			DBerror($dvdeletequery,$dvdelete);
        }
		
		$transreportquery = "INSERT INTO transactions (leagueid, teamid, dropplayerid, toir, addplayerid, fromir) VALUES($trans_league, $trans_transteam, $trans_dropplayer, $trans_putonIR, $trans_addplayer, 0)";
		$transreport = $leaguedb->query($transreportquery);
		DBerror($transreportquery,$transreport);
			
		if ($trans_dropplayer && $trans_addplayer)
		{
			drupal_set_message($teamname." dropped ".$dropplayername." and added ".$addplayername);
		}
		elseif ($trans_addplayer)
		{
			drupal_set_message($teamname." added ".$addplayername);
		}
		elseif ($trans_dropplayer)
		{
			drupal_set_message($teamname." dropped ".$dropplayername);
		}
        unset ($form_state['storage']);
    }
}
?>
