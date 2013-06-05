<?php

function fcfstransaction_all()
{
    $team = drupal_get_form('fcfstransaction_form');

    return $team;
}

function fcfstransaction_form($form_state)
{
    global $user;
    $leaguedb = connectdbs();
	$gameweek = getLeagueWeek();
    
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

	$proquery = "SELECT includepro FROM leagues WHERE leagueid = $leagueid";
	$includepro = $leaguedb->queryOne($proquery);
	DBerror($proquery, $includepro);

	$fcfstimequery = "SELECT fcfsstartday, fcfsstarttime, fcfsendday, fcfsendtime FROM leagues WHERE leagueid = $leagueid";
	$fcfstime = $leaguedb->queryRow($fcfstimequery);
	DBerror($fcfsqueryquery, $fcfstime);

	$currtime = date('l, g:i:s a', time());
	$days[1] = "Monday";
	$days[2] = "Tuesday";
	$days[3] = "Wednesday";
	$days[4] = "Thursday";
	$days[5] = "Friday";
	$days[6] = "Saturday";
	$days[7] = "Sunday";
	if (date('N',(time())) < $fcfstime['fcfsstartday'] || date('N',(time())) > $fcfstime['fcfsendday'])
	{
		drupal_set_message("It is ".$currtime." - outside of the first-come, first-served transaction period. FCFS transactions open at ".$fcfstime['fcfsstarttime']." on ".$days[$fcfstime['fcfsstartday']].". No FCFS transactions allowed at this time.");
	}
	elseif ((date('N') == $fcfstime['fcfsstartday'] && date('H:i:s',(time())) < $fcfstime['fcfsstarttime']) || (date('N') == $fcfstime['fcfsendday'] && date('H:i:s',(time())) > $fcfstime['fcfsendtime']))
	{
		drupal_set_message("It is ".$currtime." - outside of the first-come, first-served transaction period. FCFS transactions open at ".$fcfstime['fcfsstarttime']." on ".$days[$fcfstime['fcfsstartday']].". No FCFS transactions allowed at this time.");
	}
	else
	{
		$teamquery = "SELECT teamid FROM teams WHERE teamownerid = {$user->uid} and leagueid = $leagueid";
		$teamid = $leaguedb->queryOne($teamquery);
		DBerror($teamquery,$teamid);

		$playersquery = "SELECT * FROM players, playerteams WHERE players.playerid = playerteams.playerid AND teamid = $teamid order by lastname, firstname";
	   
		$playersresult = $leaguedb->query($playersquery);
		if(PEAR::isError($playersresult)) 
		{
			var_export($form_state,TRUE);
			die('Failed to issue query, error message : ' . $playersresult->getMessage()." on ".$playersquery);
		}

		//$players[0] = "Do not drop a player - add only";
		while ($playerrow = $playersresult->fetchrow(MDB2_FETCHMODE_ASSOC))
		{
			extract ($playerrow, EXTR_PREFIX_ALL, "player");
			
			$weekstart = $gameweek['Mondayprev']." 00:00:00";
			$playeraddedquery = "SELECT addplayerid FROM transactions WHERE addplayerid=$player_playerid AND teamid = $teamid AND transtime > '$weekstart'";
			$playeradded = $leaguedb->queryOne($playeraddedquery);
			
			if (!$playeradded)
			{
				$teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $player_lgteamid";
				$lgteamname = $leaguedb->queryOne($teamquery);
				$playername = $player_lastname.', '.$player_firstname.' - '.$player_pos.' - '.$lgteamname;
				$players[$player_playerid] = $playername;
			}
		}

		if ($includepro)
		{
			$nflplayersquery = "SELECT * FROM nflplayers, playerteams WHERE nflplayers.playerid = playerteams.playerid AND teamid = $teamid order by lastname, firstname";
		   
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

			$weekstart = $gameweek['Mondayprev']." 00:00:00";
			$fadroppedquery = "SELECT dropplayerid FROM transactions WHERE dropplayerid=$fa_playerid AND leagueid = $leagueid AND transtime > '$weekstart'";
			$fadropped = $leaguedb->queryOne($fadroppedquery);
			
			if (!$fadropped)
			{
				$teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $fa_lgteamid";
				$lgteamname = $leaguedb->queryOne($teamquery);
				$faname = $fa_lastname.', '.$fa_firstname.' - '.$fa_pos.' - '.$lgteamname;
				$freeagents[$fa_playerid] = $faname;
			}
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

//		$form['putonIR'] = array(
//			'#type' => 'checkbox',
//			'#title' => t('Put player on IR?'),
//		);

		$form['addplayer'] = array(
			'#type' => 'select',
			'#title' => t('Player to add'),
			'#options' => $freeagents,
		);

		$form['transteam'] = array(
			'#type' => 'hidden',
			'#value' => $teamid,
		);
		
		$form['league'] = array(
			'#type' => 'hidden',
			'#value' => $leagueid,
		);
		
		$form['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Submit transaction',
		);
	}
    return $form;
}

function fcfstransaction_form_submit($form, &$form_state)
{
	//dpm($form_state);
	//dpm($form);
	//echo "here";
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
		
		$transreportquery = "INSERT INTO transactions (leagueid, teamid, dropplayerid, toir, addplayerid, fromir) VALUES($trans_league, $trans_transteam, $trans_dropplayer, 0, $trans_addplayer, 0)";
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
