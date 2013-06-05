<?php

function starttransaction_all()
{
    global $user;
    
    $leaguedb = connectdbs();

    $userteamquery = "SELECT teamid FROM teams WHERE teamownerid = {$user->uid}";
	$userteamid = $leaguedb->queryOne($userteamquery);
    if(PEAR::isError($userteamid)) 
    {
        var_export($form_state,TRUE);
        die('Failed to issue query, error message : ' . $userteamid->getMessage()." on ".$userteamquery);
    }

    if (!isset($userteamid))
    {
        drupal_set_message('You have to be an owner of a team to submit a transaction.');
        return TRUE;
    }
    else
    {
        $team = drupal_get_form('starttransaction_form',$userteamid);
        return $team;
    }
}

function starttransaction_form($form_state,$userteamid)
{
        if (!isset($leaguedb))
        {
            $leaguedb = connectdbs();
        }
		if (isset($form_state['storage']['page_two']))
		{
			return starttransaction_form_page_2($form_state);
		}
//    elseif ($context['storage']['step'] == 3)
//    {
//        return starttransaction_form_page_3();
//    }
			
        $playersquery = "SELECT * FROM players, playerteams WHERE players.playerid = playerteams.playerid AND teamid = {$userteamid} order by lastname, firstname";
        $playersresult = $leaguedb->query($playersquery);
        if(PEAR::isError($playersresult)) 
        {
            var_export($form_state,TRUE);
            die('Failed to issue query, error message : ' . $playersresult->getMessage()." on ".$playersquery);
        }

        while ($playerrow = $playersresult->fetchrow(MDB2_FETCHMODE_ASSOC))
        {
            extract ($playerrow, EXTR_PREFIX_ALL, "player");
            $teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $player_lgteamid";
            $lgteamname = $leaguedb->queryOne($teamquery);
            $playername = $player_lastname.', '.$player_firstname.' - '.$player_pos.' - '.$lgteamname;
            $players[$player_playerid] = $playername;
        }

		$freeagents[0] = '--None';
		
        $freeagentsquery = "SELECT DISTINCT playerid, firstname, lastname, lgteamid, pos, year FROM players WHERE (NOT EXISTS (SELECT playerid FROM playerteams WHERE leagueid =1 AND players.playerid = playerteams.playerid) AND (pos = 'QB' OR pos = 'RB' OR pos = 'WR' OR pos = 'TE' OR pos = 'K')) ORDER BY lastname, firstname";
        $freeagentsresult = $leaguedb->query($freeagentsquery);

        while ($farow = $freeagentsresult->fetchrow(MDB2_FETCHMODE_ASSOC))
        {
            extract ($farow, EXTR_PREFIX_ALL, "fa");
            $teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $fa_lgteamid";
            $lgteamname = $leaguedb->queryOne($teamquery);
            $faname = $fa_lastname.', '.$fa_firstname.' - '.$fa_pos.' - '.$lgteamname;
            $freeagents[$fa_playerid] = $faname;
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
            '#type' => 'fieldset',
			'#title' => 'Players to add',
		);
		
        $form['addplayer']['add'][1] = array(
            '#type' => 'select',
            '#title' => t('First choice to add'),
            '#options' => $freeagents,
        );

        $form['addplayer']['add'][2] = array(
            '#type' => 'select',
            '#title' => t('Second choice to add'),
            '#options' => $freeagents,
        );

        $form['addplayer']['add'][3] = array(
            '#type' => 'select',
            '#title' => t('Third choice to add'),
            '#options' => $freeagents,
        );

        $form['addplayer'][4] = array(
            '#type' => 'select',
            '#title' => t('Fourth choice to add'),
            '#options' => $freeagents,
        );

        $form['addplayer'][5] = array(
            '#type' => 'select',
            '#title' => t('Fifth choice to add'),
            '#options' => $freeagents,
        );

        $form['specialinstructions'] = array(
            '#type' => 'textarea',
            '#title' => t('Special instructions for waiver'),
        );

        $form['next'] = array(
            '#type' => 'submit',
            '#value' => 'Submit transaction',
        );

        return $form;
}

function starttransaction_form_page_2($form_state)
{
	$leagueid = $form_state['values']['league'];
    if (!isset($leaguedb))
    {
        $leaguedb = connectdbs();
    }
	//dpm($form_state);
	$dropplayer = $form_state['values']['dropplayer'];
	$addcount = 1;
	for ($formkey == 1;$formkey<=5;$formkey++)
	{
		if ($form_state['values'][$formkey] != 0)
		{
			$addplayers[$addcount] = $form_state['values'][$formkey];
			$addcount++;
		}
	}
	dpm($addplayers);
}

function starttransaction_form_submit($form, &$form_state)
{
    if (!isset($leaguedb))
    {
        $leaguedb = connectdbs();
    }
    if ($form_state['clicked_button']['#id'] == 'edit-next')
    {
        $form_state['storage']['page_two'] = TRUE;
		
    }
	else
	{
		$addplayername = getPlayerName($form_state['values']['addplayer'],$leaguedb);
		$dropplayername = getPlayerName($form_state['values']['dropplayer'],$leaguedb);
		if ($form_state['values']['putonIR'])
		{
			$dropmessage = ' put '.$dropplayername.' on IR.';
		}
		else
		{
			$dropmessage = ' dropped '.$dropplayername.'.';
		}
	//    drupal_set_message('<pre>'. var_export($form_state['values'],TRUE) .'</pre>');
		drupal_set_message('You added '.$addplayername.' and '.$dropmessage);
		$form_state['redirect'] = '';
	}
}
?>