<?php

function playeredit_all()
{
    $team = drupal_get_form('playeredit_form');

    return $team;
}

function playeredit_form($form_state)
{
    if (isset($form_state['storage']['page_two']))
    {
        return playeredit_form_page_2($form_state);
    }
//    elseif ($context['storage']['step'] == 3)
//    {
//        return starttransaction_form_page_3();
//    }
    
    $chosenleague = 1;

    $leaguedb = connectdbs();
    $teamsquery = "SELECT lgteamid, lgteamname FROM lgteams WHERE lgteamid < 121 ORDER BY lgteamname;";
    $teamsresult = $leaguedb->query($teamsquery );

    while ($team = $teamsresult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
        $teamsarray[$team['lgteamid']] = $team['lgteamname'];
        //print_r($team);
    }

    //print_r($teamsarray);
    $form['team'] = array(
        '#type' => 'select',
        '#title' => t('Team to list players for'),
        '#options' => $teamsarray,
    );
    
    $form['next'] = array(
        '#type' => 'submit',
        '#value' => 'List players on this team',
    );

    return $form;
}

function playeredit_form_page_2($form_state)
{
	$chosenleague = 1;
    //print_r($form_state);
	
    if (!isset($leaguedb))
    {
        $leaguedb = connectdbs();
    }
    $playersquery = "SELECT * FROM players WHERE players.lgteamid = {$form_state['values']['team']} AND (pos='QB' OR pos='RB' OR pos='WR' OR pos='TE' OR pos='D' or pos='') order by lastname, firstname";
	$playersresult = $leaguedb->query($playersquery);
    if(PEAR::isError($playersresult)) 
    {
        var_export($form_state,TRUE);
        die('Failed to issue query, error message : ' . $playersresult->getMessage()." on ".$playersquery);
    }

	$positions = array('','QB','RB','WR','TE','K');
	$years = array('','FR','SO','JR','SR','Rook');
	
   	while ($playerrow = $playersresult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
		extract ($playerrow, EXTR_PREFIX_ALL, "player");
        $players[$player_playerid]['lastname'] = $player_lastname;
        $players[$player_playerid]['firstname'] = $player_firstname;
        $players[$player_playerid]['pos'] = $player_pos;
        $players[$player_playerid]['year'] = $player_year;

		$form[$player_playerid]['lastname'] = array(
			'#type' => 'textfield',
			'#title' => t('Last Name'),
			'#default_value' => $players[$player_playerid]['lastname'],
		);

		$form[$player_playerid]['firstname'] = array(
			'#type' => 'textfield',
			'#title' => t('First Name'),
			'#default_value' => $players[$player_playerid]['firstname'],
		);

		$form[$player_playerid]['position'] = array(
			'#type' => 'select',
			'#title' => t('Position'),
			'#options' => $positions,
			'#default_value' => array_search($players[$player_playerid]['pos'], $positions),
		);

		$form[$player_playerid]['year'] = array(
			'#type' => 'select',
			'#title' => t('Year'),
			'#options' => $years,
			'#default_value' => array_search($players[$player_playerid]['year'], $years),
		);

		$form['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Submit edits',
		);

    }
	return $form;
}

function playeredit_form_submit($form, &$form_state)
{

    if (!isset($leaguedb))
    {
        $leaguedb = connectdbs();
    }

    if ($form_state['clicked_button']['#id'] == 'edit-next')
    {
        $form_state['storage']['page_two'] = TRUE;
        //$form_state['storage']['teamid'] = $form_state['values']['team'];
    }
    else
    {
		extract ($form_state['values'], EXTR_PREFIX_ALL, "players");
        print_r($form_state['values']);
/*
		$addplayername = getPlayerName($trans_addplayer, $leaguedb);
		if ($trans_dropplayer)
		{
			$dropplayername = getPlayerName($trans_dropplayer, $leaguedb);
		}
		
		$teamnamequery = "SELECT teamfirstname FROM teams WHERE teamid=$trans_transteam";
		$teamname = $leaguedb->queryOne($teamnamequery);

		if ($trans_dropplayer && !$trans_putonIR)
		{
			$transdropquery = "DELETE FROM playerteams WHERE playerid = $trans_dropplayer AND leagueid = $chosenleague";
			$transdrop = $leaguedb->query($transdropquery);
			DBerror($transdropquery,$transdrop);
		}
		
		$transaddquery = "INSERT INTO playerteams VALUES($trans_addplayer, $trans_league, $trans_transteam, NULL)";
		$transadd = $leaguedb->query($transaddquery);
		DBerror($transaddquery,$transadd);
		
		$transreportquery = "INSERT INTO transactions (leagueid, teamid, dropplayerid, toir, addplayerid, fromir) VALUES($trans_league, $trans_transteam, $trans_dropplayer, $trans_putonIR, $trans_addplayer, 0)";
		$transreport = $leaguedb->query($transreportquery);
		DBerror($transreportquery,$transreport);
			
		if ($transdropplayer)
		{
			drupal_set_message($teamname." dropped ".$dropplayername." and added ".$addplayername);
		}
		else
		{
			drupal_set_message($teamname." added ".$addplayername);
		}
        unset ($form_state['storage']);*/
    }
}
?>