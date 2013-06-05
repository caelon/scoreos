<?php

function setlineup_all()
{
    $team = drupal_get_form('setlineup_form');
	drupal_set_message('Efforts are made to make sure the schedules are accurate but each owner is responsible for verifying.');

    return $team;
}

function theme_setlineup_form($form)
{
	$currpos = '';
    //dpm("This page is currently being worked on. Apologies if you are trying to use it.");
    //dpm("Please e-mail me at caelon@gmail.com if you need to use it right now.");
	$header = array();
    //dpm($form);
	foreach (element_children($form) as $key)
	{
        $row = array();
        if (isset($form[$key]['lastname']))
        {
			if ($currpos != $form[$key]['pos']['#value'])
			{
				if ($header)
				{
					$output .= theme('table', $header, $rows);
					$header = array();
					$rows = array();
				}
				$currpos = $form[$key]['pos']['#value'];
                $header[] = array('data' => $currpos, 'style' => "width: 50%");
                $header[] = array('data' => 'Opponent', 'style' => "width: 20%");
                $header[] = array('data' => 'Game Time (Eastern)', 'style' => "width: 20%");
                $header[] = array('data' => 'Starter', 'style' => "width: 10%");
                $row = array();
			}
            
            if(!$form['starter'][$key]['#value'])
            {
                $class = 'starter';
                $style = 'color: blue;';
            }
            else
            {
                $class = 'bench';
                $style = '';
            }
            
            $starter = drupal_render($form['starter'][$key]);
            
            if ($currpos == 'D')
            {
       			$row[] = array('data' =>drupal_render($form[$key]['firstname']).' '.drupal_render($form[$key]['lastname']), 'class' => $class, 'style' => $style);
                $erasedefteam = drupal_render($form[$key]['teamname']);
            }
            else
            {
       			$row[] = array('data' =>drupal_render($form[$key]['lastname']).', '.drupal_render($form[$key]['firstname']).' - '.drupal_render($form[$key]['teamname']), 'class' => $class, 'style' => $style);
            }
			$row[] = array('data' => drupal_render($form[$key]['opponent']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['gametime']), 'class' => $class, 'style' => $style);
 
			$row[] = array('data' => $starter, 'class' => 'select');
            
			$rows[] = $row;
		}
	}
 
	$output .= theme('table', $header, $rows);
	$output .= drupal_render($form);
	return $output;
}

function setlineup_form($form_state)
{
	$gameweek = getLeagueWeek(16); //set to 16 for playoff
//	$gameweek = getLeagueWeek();
	
    global $user;
    $leaguedb = connectdbs();
    if (isset($form_state['storage']['submit']))
    {
		echo "here?";
        //$lineup_message = setlineup_submitchanges($form_state);
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
	$leagueidquery = "SELECT leagueid, lgteaswr, tiebreaker, includepro FROM leagues WHERE leagueurl = '$leagueurl'";
	$leaguerow = $leaguedb->queryRow($leagueidquery);
	DBerror($leagueidquery, $leaguerow);
    
    extract ($leaguerow);
    
	if (!$tiebreaker)
	{
		$starterlisting = array('Starter','Bench');
	}
	else
	{
		$starterlisting = array('Starter','Bench','Tie Breaker');
	}
	
	$teamid = arg(2);
	if (isset($teamid))
	{
		$userquery = "SELECT teamownerid FROM teams WHERE teamid = $teamid and leagueid = $leagueid";
		$userid = $leaguedb->queryOne($userquery);
		DBerror($userquery,$userid);
		
		if (is_array($user->roles))
		{
			foreach($user->roles as $role)
			{
				if ($role == 'Commissioner')
				{
					$admin = 1;
				}
			}
		}
		
		if ($user->uid == 1)
		{
			$admin = 1;
		}
        
		if ($userid != $user->uid && !$admin)
		{
		    drupal_set_message('You were trying to set lineups for a team that is not yours. Please try the Set Lineup feature again.');
		    drupal_goto('');
		}
		elseif ($userid != $user->uid && $admin)
		{
			drupal_set_message('You are altering someone else\'s lineup.');
		}
	}
	else
	{
		$teamquery = "SELECT teamid FROM teams WHERE teamownerid = {$user->uid} and leagueid = $leagueid";
		$teamid = $leaguedb->queryOne($teamquery);
		DBerror($teamquery,$teamid);
	}
	
	$playersquery = "SELECT * FROM players, playerteams, positionorder WHERE players.playerid = playerteams.playerid AND playerteams.teamid = $teamid AND playerteams.leagueid = $leagueid AND positionorder.pos = players.pos AND positionorder.leagueid = $leagueid AND irweek IS NULL ORDER BY posorder, lastname, firstname";
	$playersresult = $leaguedb->query($playersquery);
    DBerror($playersquery,$playersresult);

        if ($includepro)
    	{
        	$proplayersquery = "SELECT * FROM nflplayers, playerteams, positionorder WHERE nflplayers.playerid = playerteams.playerid AND playerteams.teamid = $teamid AND playerteams.leagueid = $leagueid AND positionorder.pos = nflplayers.pos AND positionorder.leagueid = $leagueid AND irweek IS NULL ORDER BY posorder, lastname, firstname";
        	$proplayersresult = $leaguedb->query($proplayersquery);
        	DBerror($proplayersquery,$proplayersresult);
	}
    
	$checklineupquery = "SELECT playerid, starter FROM playerlineups WHERE teamid = $teamid AND week = {$gameweek['week']}";
	$thisweeklineup = $leaguedb->query($checklineupquery);
	DBerror($checklineupquery,$thisweeklineup);
		
    $pastgametime = array();
    
   	while ($playerrow = $playersresult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
		extract ($playerrow, EXTR_PREFIX_ALL, "player");
		$teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $player_lgteamid";
		$lgteamname = $leaguedb->queryOne($teamquery);
		DBerror($teamquery,$lgteamname);

		$lineup[$player_playerid]['teamname'] = $lgteamname;
        $lineup[$player_playerid]['collegepro'] = 'college';
		
		if ($thisweeklineup)
		{
			$lineupquery = "SELECT starter FROM playerlineups WHERE playerid = $player_playerid AND teamid = $teamid AND week = {$gameweek['week']}";
		}
		else
		{
			$lineupquery = "SELECT starter FROM playerlineups WHERE playerid = $player_playerid AND teamid = $teamid AND week = {$gameweek['lastweek']}";
		}
		
		$playerstart = $leaguedb->queryOne($lineupquery);
		DBerror($lineupquery,$playerstart);
		
		if ($playerstart == 1)
		{
			$lineup[$player_playerid]['start'] = 0;
		}
        elseif ($playerstart == 2)
		{
			$lineup[$player_playerid]['start'] = 2;
		}
		else
		{
			$lineup[$player_playerid]['start'] = 1;
		}
        
        if ($leagueid == 1 || $leagueid == 2 || $leagueid == 3 || $leagueid == 5)
        {
            $weekstart = $gameweek['Tuesdayprev'];
            $weekend = $gameweek['Monday'];
        }
        else
        {
            $weekstart = $gameweek['Wednesday'];
            $weekend = $gameweek['Tuesday'];
        }

        //bowl season
        $weekstart = '20121215';
        $weekend = '20130106';
        
		$oppquery = "SELECT * FROM ncaasched WHERE (visitor = $player_lgteamid OR home = $player_lgteamid) AND gamedate >= '$weekstart' AND gamedate <= '$weekend'";
		$opponent = $leaguedb->queryRow($oppquery);
		DBerror($oppquery, $opponent);
		
        $location = 'BYE';
        $oppname = '';
		if ($opponent['visitor'] == $player_lgteamid)
		{
			$opp = $opponent['home'];
			$location = 'at ';
		}
		elseif ($opponent['home'] == $player_lgteamid)
		{
			$opp = $opponent['visitor'];
			$location = 'vs ';
		}
        
        if ($location != 'BYE')
        {

            $oppteamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $opp";
            $oppname = $leaguedb->queryOne($oppteamquery);
            DBerror($oppteamquery, $oppname);
		}
        
        if ($location != 'BYE')
        {
            $gametime = mktime(substr($opponent['gametime'],0,2),substr($opponent['gametime'],3,2),0,substr($opponent['gamedate'],5,2),substr($opponent['gamedate'],8,2),substr($opponent['gamedate'],0,4));
            if ($gametime <= time())
            {
                $pastgametime[] = $player_playerid;
            }
            $gamedatestring = date("D M j g:i A", $gametime);
        }
        else
        {
            $gamedatestring = '---';
        }
		
		$lineup[$player_playerid]['location'] = $location;
		$lineup[$player_playerid]['oppname'] = $oppname;
		$lineup[$player_playerid]['gamedatestring'] = $gamedatestring;
		$lineup[$player_playerid]['lastname'] = $player_lastname;
		$lineup[$player_playerid]['firstname'] = $player_firstname;
		$lineup[$player_playerid]['pos'] = $player_pos;
		$lineup[$player_playerid]['posorder'] = $player_posorder;
		$lineup[$player_playerid]['playerid'] = $player_playerid;
	}

    if ($includepro)
    {
        while ($playerrow = $proplayersresult->fetchrow(MDB2_FETCHMODE_ASSOC))
        {
            extract ($playerrow, EXTR_PREFIX_ALL, "player");
            $teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $player_lgteamid";
            $lgteamname = $leaguedb->queryOne($teamquery);
            DBerror($teamquery,$lgteamname);

			$lineup[$player_playerid]['teamname'] = $lgteamname;
            $lineup[$player_playerid]['collegepro'] = 'pro';
            
            if ($thisweeklineup)
            {
                $lineupquery = "SELECT starter FROM playerlineups WHERE playerid = $player_playerid AND teamid = $teamid AND week = {$gameweek['week']}";
            }
            else
            {
                $lineupquery = "SELECT starter FROM playerlineups WHERE playerid = $player_playerid AND teamid = $teamid AND week = {$gameweek['lastweek']}";
            }
            
            $playerstart = $leaguedb->queryOne($lineupquery);
            DBerror($lineupquery,$playerstart);
            
            if ($playerstart == 1)
            {
                $lineup[$player_playerid]['start'] = 0;
            }
            elseif ($playerstart == 2)
            {
                $lineup[$player_playerid]['start'] = 2;
            }
            else
            {
                $lineup[$player_playerid]['start'] = 1;
            }
            
    		$oppquery = "SELECT * FROM nflsched WHERE (visitor = $player_lgteamid OR home = $player_lgteamid) AND gamedate >= '{$gameweek['Tuesdayprev']}' AND gamedate <= '{$gameweek['Tuesday']}'";
    		$opponent = $leaguedb->queryRow($oppquery);
    		DBerror($oppquery, $opponent);
            
            $location = 'BYE';
            $oppname = '';
            if ($opponent['visitor'] == $player_lgteamid)
            {
                $opp = $opponent['home'];
                $location = 'at ';
            }
            elseif ($opponent['home'] == $player_lgteamid)
            {
                $opp = $opponent['visitor'];
                $location = 'vs ';
            }
            
            if ($location != 'BYE')
            {
                $oppteamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $opp";
                $oppname = $leaguedb->queryOne($oppteamquery);
                DBerror($oppteamquery, $oppname);
            }
            
            if ($location != 'BYE')
            {
                $gametime = mktime(substr($opponent['gametime'],0,2),substr($opponent['gametime'],3,2),0,substr($opponent['gamedate'],5,2),substr($opponent['gamedate'],8,2),substr($opponent['gamedate'],0,4));
                if ($gametime <= (time()-(4*60*60)))
                {
                    $pastgametime[] = $player_playerid;
                }
                $gamedatestring = date("D M j g:i A", $gametime);
            }
            else
            {
                $gamedatestring = '---';
            }
            
            $lineup[$player_playerid]['location'] = $location;
            $lineup[$player_playerid]['oppname'] = $oppname;
            $lineup[$player_playerid]['gamedatestring'] = $gamedatestring;
            $lineup[$player_playerid]['lastname'] = $player_lastname;
            $lineup[$player_playerid]['firstname'] = $player_firstname;
            $lineup[$player_playerid]['pos'] = $player_pos;
            $lineup[$player_playerid]['posorder'] = $player_posorder;
            $lineup[$player_playerid]['playerid'] = $player_playerid;
        }
    }
    
    $sortedlineup = array_csort($lineup,'collegepro','posorder','start','lastname');

	foreach ($sortedlineup as $index => $lineupplayer)
	{
		//dpm($playerid);
		$form[$lineupplayer['playerid']]['lastname'] = array(
			'#type' => 'markup',
			'#value' => $lineupplayer['lastname'],
		);
		
		$form[$lineupplayer['playerid']]['firstname'] = array(
			'#type' => 'markup',
			'#value' => $lineupplayer['firstname'],
		);
		
        if ($lineupplayer['pos'] == 'TE' && $lgteaswr && !$includepro)
        {
            $lineupplayer['pos'] = 'WR';
        }
        
		$form[$lineupplayer['playerid']]['pos'] = array(
			'#type' => 'value',
			'#value' => $lineupplayer['pos'],
		);
		
		$form[$lineupplayer['playerid']]['teamname'] = array(
			'#type' => 'markup',
			'#value' => $lineupplayer['teamname'],
		);
		
		$form[$lineupplayer['playerid']]['opponent'] = array(
			'#type' => 'markup',
			'#value' => $lineupplayer['location'].$lineupplayer['oppname'],
		);
		
		$form[$lineupplayer['playerid']]['gametime'] = array(
			'#type' => 'markup',
			'#value' => $lineupplayer['gamedatestring'],
		);
		
		$form[$lineupplayer['playerid']]['playerid'] = array(
			'#type' => 'value',
			'#value' => $lineupplayer['playerid'],
        );

		$form[$lineupplayer['playerid']]['collegepro'] = array(
			'#type' => 'value',
			'#value' => $lineupplayer['collegepro'],
        );

        if (in_array($lineupplayer['playerid'],$pastgametime) && $admin != 1)
        {
            $form['starter'][$lineupplayer['playerid']] = array(
                '#type' => 'select',
                '#default_value' => $lineupplayer['start'],
                '#options' => $starterlisting,
                '#disabled' => TRUE,
            );
        }
        else
        {
            $form['starter'][$lineupplayer['playerid']] = array(
                '#type' => 'select',
                '#default_value' => $lineupplayer['start'],
                '#options' => $starterlisting,
            );
        }
        
    }
    $form['week'] = array(
        '#type' => 'value',
        '#value' => $gameweek['week'],
    );
		
    $form['leagueid'] = array(
        '#type' => 'value',
        '#value' => $leagueid,
    );
    
    $form['teamid'] = array(
        '#type' => 'value',
        '#value' => $teamid,
    );
		
	$form['next'] = array(
		'#type' => 'submit',
		'#value' => 'Submit Lineup',
	);
	
    return $form;
}

function setlineup_form_validate($form, &$form_state)
{
    if (!isset($leaguedb))
    {
        $leaguedb = connectdbs();
    }

	$starter_values = $form['starter'];
    $starters = array();
    
	foreach($starter_values as $key => $value)
	{
        //dpm($form[$key]['collegepro']['#value']);

		if (is_numeric($key))
        {
            if (!$value['#value'])
            {
                $starters[$form[$key]['collegepro']['#value']][$form[$key]['pos']['#value']] += 1 ;
                //dpm($form[$key]['collegepro']['#value']);
                if ($form[$key]['collegepro']['#value'] == 'pro') //check to see if there are any pro players
                {
                    $includepro = 1;
                }
            }
        }
    }

    $posquery = "SELECT * FROM positionorder WHERE leagueid = {$form['leagueid']['#value']} AND parentpos IS NULL ORDER BY posorder";
    $posresult = $leaguedb->query($posquery);
    DBerror($posquery, $posresult);
		
    $poscheck = array();
    
   	while ($posrow = $posresult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
        extract ($posrow, EXTR_PREFIX_ALL, "posreq");

        if (!isset($starters['college'][$posreq_pos]))
        {
            $starters['college'][$posreq_pos] = 0;
        }
        
        $poscheck['college'][$posreq_pos]['min'] += $posreq_minimum;
        $poscheck['college'][$posreq_pos]['max'] += $posreq_maximum;

        if ($posreq_flexgroup && $posreq_pos != 'FLX')
        {
            
            $poscheck['college']['FLX']['starters'] += $starters['college'][$posreq_pos];
            if ($poscheck['college']['FLX']['total'])
            {
                $poscheck['college']['FLX']['total'] += $posreq_minimum;
            }
            else
            {
                $poscheck['college']['FLX']['total'] = $posreq_maximum;
            }
            
            $poscheck['college']['FLX']['min'] += $posreq_minimum;
        }
    }
    
    if ($includepro)
    {
        $poscheck['pro'] = $poscheck['college'];
        dpm($poscheck['pro']);
    }

    foreach ($poscheck['college'] as $poskey => $posvalue)
    {
        if (($starters['college'][$poskey] < $posvalue['min'] || $starters['college'][$poskey] > $posvalue['max']) && $poskey != 'FLX')
        {
            if ($posvalue['min'] == $posvalue['max'])
            {
                if ($posvalue['min'] == 1)
                {
                    $message = 'You submitted '.$starters['college'][$poskey].' '.$poskey.'s. '.$posvalue['min'].' is required.';
                }
                else
                {
                    $message = 'You submitted '.$starters['college'][$poskey].' '.$poskey.'. '.$posvalue['min'].' are required.';
                }
            }
            else
            {
                $message = 'You submitted '.$starters['college'][$poskey].' '.$poskey.'. '.$posvalue['min'].'-'.$posvalue['max'].' are required.';
            }
            $message .= ' Lineup was not saved. Please submit a legal lineup.';
            form_set_error('starter',t($message));
        }
        
        if ($poskey == 'FLX')
        {
            if ($posvalue['starters'] > $posvalue['total'])
            {
                $message = 'You have too many flex players. Lineup was not saved. Please submit a legal lineup.';
                form_set_error('starter',t($message));
            }
            elseif ($posvalue['starters'] < $posvalue['total'])
            {
                $message = 'You have not assigned a flex player. Lineup was not saved. Please select one more starter and submit a legal lineup.';
                form_set_error('starter',t($message));
            }
        }
    }
}

function setlineup_form_submit($form, &$form_state)
{
    if (!isset($leaguedb))
    {
        $leaguedb = connectdbs();
    }

	$starter_values = $form['starter'];
	$starter_array = $starter_values['starter'];
	
	extract ($starter_values, EXTR_PREFIX_ALL, "lineup");

    //echo '<pre>';
    //echo '</pre>';
	
	//dpm($starter_values);
	
	$clearlineupquery = "DELETE FROM playerlineups WHERE week={$form['week']['#value']} AND teamid={$form['teamid']['#value']}";
	$clearlineupresult = $leaguedb->query($clearlineupquery);
	DBerror($clearlineupquery, $clearlineupresult);
	
	$starters = array();
	
	foreach($starter_values as $key => $value)
	{
		if (is_numeric($key))
        {
            if ($value['#value'] == 1)
            {
                $starter = 0;
            }
            elseif ($value['#value'] == 2)
            {
                $starter = 2;
            }
			else
            {
                $starter = 1;
            }

            $addlineupquery = "INSERT INTO playerlineups VALUES ({$form['week']['#value']}, $key, '$starter', 0, {$form['teamid']['#value']})";
            $addlineupresult = $leaguedb->query($addlineupquery);
            DBerror($addlineupquery, $addlineupresult);	
        }
    }
	
	drupal_set_message("Lineup change saved.");

	unset ($form_state['storage']);
}

function player_pastgametime($form, $edit) {
  	foreach ($form['#pastgametime'] as $key) {
  		$form['starter'][$key]['#attributes'] = array('disabled' => 'TRUE');
        dpm($form['starter'][$key]);
  	}
 
  	return $form;
}
?>
