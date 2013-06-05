<?php

function playersearch_all()
{
    $team = drupal_get_form('playersearch_form');

    return $team;
}

function theme_playersearch_form($form)
{
    //dpm($form);
    $style = "text-align: center;";
	
    //$filterrow = array();
    $filterheader = array('data' => t('Positions'),t('Conferences'),t('Team'),t('Last Name'));
    $filterrow[] = array('data' => drupal_render($form['poschoice']));
    $filterrow[] = array('data' => drupal_render($form['confchoice']));
    $filterrow[] = array('data' => drupal_render($form['teamchoice']));
    $filterrow[] = array('data' => drupal_render($form['namefilter']));
    $filterrows[] = $filterrow;
	$output = theme('table', $filterheader, $filterrows);
	$output .= drupal_render($form['filter']);
	
	foreach (element_children($form) as $key)
	{
        $row = array();
        if (isset($form[$key]['lastname']))
        {
            $currpos = $form[$key]['pos']['#value'];
            //$addplayerbox = drupal_render($form['addplayer'][$key]);
            
			//$row[] = array('data' => $addplayerbox, 'class' => 'checkbox');
			$row[] = array('data' => drupal_render($form[$key]['pos']), 'class' => $class, 'style' => $style);
   			$row[] = array('data' =>drupal_render($form[$key]['lastname']).', '.drupal_render($form[$key]['firstname']).'<br />'.drupal_render($form[$key]['teamname']).' ('.drupal_render($form[$key]['year']).')');
			$row[] = array('data' => drupal_render($form[$key]['carries']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['rushyds']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['rushtds']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['recepts']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['recyds']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['rectds']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['completions']).'-'.drupal_render($form[$key]['attempts']).'-'.drupal_render($form[$key]['passints']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['passyds']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['passtds']), 'class' => $class, 'style' => $style);
			$row[] = array('data' => drupal_render($form[$key]['points']), 'class' => $class, 'style' => $style);
			//$row[] = array('data' => drupal_render($form[$key]['puntretyds']), 'class' => $class, 'style' => $style);
			//$row[] = array('data' => drupal_render($form[$key]['puntrettds']), 'class' => $class, 'style' => $style);
			//$row[] = array('data' => drupal_render($form[$key]['kickretyds']), 'class' => $class, 'style' => $style);
			//$row[] = array('data' => drupal_render($form[$key]['kickrettds']), 'class' => $class, 'style' => $style);

			$rows[] = $row;
		}
	}
 
	$header = array();
    //$header[] = array('data' => t('Add Player'), 'class' => 'checkbox');
    $header[] = t('Pos');
    $header[] = t('Player');
    //$header[] = t('Rushing');
    $header[] = t('Rush');
    $header[] = t('Rush<br />Yds');
    $header[] = t('Rush<br />TDs');
    //$header[] = t('Receiving');
    $header[] = t('Rec');
    $header[] = t('Rec<br />Yds');
    $header[] = t('Rec<br />TDs');
    //$header[] = t('Passing');
    $header[] = t('Comp-Att-<br />Int');
    $header[] = t('Pass<br />Yds');
    $header[] = t('Pass<br />TDs');
    $header[] = t('Pts');
    //$header[] = t('PR Yds');
    //$header[] = t('PR TDs');
    //$header[] = t('KR Yds');
    //$header[] = t('KR TDs');
 
	$output .= theme('table', $header, $rows);
	$output .= drupal_render($form);
	return $output;
}

function playersearch_form($form_state)
{
	$gameweek = getLeagueWeek();

    global $user;
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
	$leagueidquery = "SELECT leagueid, lgteaswr FROM leagues WHERE leagueurl = '$leagueurl'";
	$leaguerow = $leaguedb->queryRow($leagueidquery);
	DBerror($leagueidquery, $leaguerow);
    
    $leagueid = $leaguerow['leagueid'];
    $lgteaswr = $leaguerow['lgteaswr'];
    
	$scoring=array();
	$scoring=getScoringSystem($leaguedb, $leagueid);
	
	//$teamid = arg(2);
	if (isset($teamid))
	{
		$teamid = arg(2);
	}
	else
	{
		$teamquery = "SELECT teamid FROM teams WHERE teamownerid = {$user->uid} and leagueid = $leagueid";
		$teamid = $leaguedb->queryOne($teamquery);
		DBerror($teamquery,$teamid);
	}

	$posquery = "SELECT pos, posorder FROM positionorder WHERE leagueid = $leagueid ORDER by posorder";
	$posresult = $leaguedb->query($posquery);
	DBerror($posquery,$posresult);
	
	$posoptions = array();
	$posoptions[] = 'All';
	while ($posrow = $posresult->fetchrow(MDB2_FETCHMODE_ASSOC))
	{
		if ($posrow['pos'] != 'FLX')
		{
			$posoptions[$posrow['posorder']]=$posrow['pos'];
		}
        if (($leagueid == 3 || $leagueid ==5) && $posrow['pos'] == 'TE')
        {
			$posoptions[4]='TE';
        }
	}
	
	$confquery = "SELECT ncaaconferenceid, ncaaconferenceabbr FROM ncaaconferences";
	$confresult = $leaguedb->query($confquery);
	DBerror($confquery,$confresult);
	
	$confoptions = array();
	$confoptions[] = 'All';
	while ($confrow = $confresult->fetchrow(MDB2_FETCHMODE_ASSOC))
	{
		$confoptions[$confrow['ncaaconferenceid']]=$confrow['ncaaconferenceabbr'];
	}
	
	$teamquery = "SELECT lgteamid, lgteamname FROM lgteams WHERE lgteamid < 121 ORDER BY lgteamname";
	$teamresult = $leaguedb->query($teamquery);
	DBerror($teamquery,$teamresult);
	
	$teamoptions = array();
	$teamoptions[] = 'All';
	while ($teamrow = $teamresult->fetchrow(MDB2_FETCHMODE_ASSOC))
	{
		$teamoptions[$teamrow['lgteamid']]=$teamrow['lgteamname'];
	}
	
	$posfilter = '';
	$namefilter = '';
    $conffilter = '';
    $teamfilter = '';
    $conftable = '';
    
	if ($form_state['rebuild'] == TRUE && isset($form_state['storage']))
	{
		if (isset($form_state['storage']['stored_pos']))
		{
			foreach ($form_state['storage']['stored_pos'] as $filterpos)
			{
				if ($posoptions[$filterpos] != 'All')
				{
					if (!$posfilter)
					{
						$posfilter = "AND (players.pos = '".$posoptions[$filterpos]."'";
					}
					else
					{
						$posfilter .= " OR players.pos = '".$posoptions[$filterpos]."'";
					}
				}
			}
			if (!empty($posfilter))
			{
				$posfilter .= ")";
			}
		}
		if (isset($form_state['storage']['stored_conf']))
		{
			foreach ($form_state['storage']['stored_conf'] as $filterconf)
			{
				if ($confoptions[$filterconf] != 'All')
				{
                    $conftable = ", lgteams";
					if (!$conffilter)
					{
						$conffilter = "AND players.lgteamid = lgteams.lgteamid AND (lgteams.lgconferenceid = ".$filterconf;
					}
					else
					{
						$conffilter .= " OR lgteams.lgconferenceid = ".$filterconf;
					}
				}
			}
			if (!empty($conffilter))
			{
				$conffilter .= ")";
			}
		}
		if (isset($form_state['storage']['stored_team']))
		{
			$filterteam = $form_state['storage']['stored_team'];
            if ($teamoptions[$filterteam] != 'All')
            {
                if (!$conftable)
                {
                    $teamtable = ", lgteams";
                }
                $teamfilter = "AND players.lgteamid = lgteams.lgteamid AND lgteams.lgteamid = ".$filterteam;
            }
		}
		if (strlen($form_state['storage']['stored_name']) > 0)
		{
			$namefilter = "AND players.lastname LIKE '{$form_state['storage']['stored_name']}%'";
		}
	}
	
    $freeagentsquery = "SELECT playerid, firstname, lastname, players.lgteamid, players.pos, year FROM players, positionorder $conftable $teamtable WHERE playerid NOT IN (SELECT playerteams.playerid FROM playerteams, positionorder, players WHERE playerteams.leagueid = $leagueid AND players.playerid = playerteams.playerid AND players.pos = positionorder.pos and positionorder.leagueid = playerteams.leagueid) AND players.pos = positionorder.pos and positionorder.leagueid = $leagueid AND players.lgteamid <= 120 AND (year != 'Rook' and year != 'Pro') $posfilter $namefilter $conffilter $teamfilter ORDER BY lastname, firstname";
	$freeagentsresult = $leaguedb->query($freeagentsquery);
    DBerror($freeagentsquery,$freeagentsresult);

   	while ($playerrow = $freeagentsresult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
		extract ($playerrow, EXTR_PREFIX_ALL, "player");
        $freeagent[$player_playerid]['playerid'] = $player_playerid;
        $freeagent[$player_playerid]['firstname'] = $player_firstname;
        $freeagent[$player_playerid]['lastname'] = $player_lastname;

        if ($player_pos == 'TE' && $lgteaswr && ($leagueid != 3 && $leagueid !=5))
        {
            $player_pos = 'WR';
        }
        
        $freeagent[$player_playerid]['pos'] = $player_pos;
        $freeagent[$player_playerid]['year'] = $player_year;
        $freeagent[$player_playerid]['teamid'] = $player_lgteamid;
		$teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $player_lgteamid";
		$freeagent[$player_playerid]['teamname'] = $leaguedb->queryOne($teamquery);
		//$freeagent[$player_playerid]['teamname'] = getTeamAbbr($leaguedb, $player_lgteamid);
		DBerror($teamquery,$freeagent[$player_playerid]['teamname']);
		
        $playerstatquery = "SELECT week, xp2, puntretyds, puntrettds, kickretyds, kickrettds, carries, rushyds, rushtds, completions, attempts, passints, passyds, passtds, recepts, recyds, rectds FROM playerstats WHERE playerid = $player_playerid";
		$playerstatresult = $leaguedb->query($playerstatquery);
		DBerror($playerstatquery,$playerstatresult);
		
        while ($playerstatrow = $playerstatresult->fetchrow(MDB2_FETCHMODE_ASSOC))
        {
            extract ($playerstatrow, EXTR_PREFIX_ALL, "playerstat");
            $freeagent[$player_playerid]['xp2'] += $playerstat_xp2;
            $freeagent[$player_playerid]['xp2wk'][$playerstat_week] = $playerstat_xp2;
            $freeagent[$player_playerid]['puntretyds'] += $playerstat_puntretyds;
            //$freeagent[$player_playerid]['puntretydswk'][$playerstat_week] = $playerstat_puntretyds;
            $freeagent[$player_playerid][$playerstat_week]['puntretyds'] = $playerstat_puntretyds;
            $freeagent[$player_playerid]['puntrettds'] += $playerstat_puntrettds;
            //$freeagent[$player_playerid]['puntrettdswk'][$playerstat_week] = $playerstat_puntrettds;
            $freeagent[$player_playerid][$playerstat_week]['puntrettdswk'] = $playerstat_puntrettds;
            $freeagent[$player_playerid]['kickretyds'] += $playerstat_kickretyds;
            //$freeagent[$player_playerid]['kickretydswk'][$playerstat_week] = $playerstat_kickretyds;
            $freeagent[$player_playerid][$playerstat_week]['kickretydswk'] = $playerstat_kickretyds;
            $freeagent[$player_playerid]['kickrettds'] += $playerstat_kickrettds;
            //$freeagent[$player_playerid]['kickrettdswk'][$playerstat_week] = $playerstat_kickrettds;
            $freeagent[$player_playerid][$playerstat_week]['kickrettdswk'] = $playerstat_kickrettds;
            $freeagent[$player_playerid]['carries'] += $playerstat_carries;
            //$freeagent[$player_playerid]['carrieswk'][$playerstat_week] = $playerstat_carries;
            $freeagent[$player_playerid][$playerstat_week]['carrieswk'] = $playerstat_carries;
            $freeagent[$player_playerid]['rushyds'] += $playerstat_rushyds;
            //$freeagent[$player_playerid]['rushydswk'][$playerstat_week] = $playerstat_rushyds;
            $freeagent[$player_playerid][$playerstat_week]['rushydswk'] = $playerstat_rushyds;
            $freeagent[$player_playerid]['rushtds'] += $playerstat_rushtds;
            //$freeagent[$player_playerid]['rushtdswk'][$playerstat_week] = $playerstat_rushtds;
            $freeagent[$player_playerid][$playerstat_week]['rushtdswk'] = $playerstat_rushtds;
            $freeagent[$player_playerid]['completions'] += $playerstat_completions;
            //$freeagent[$player_playerid]['completionswk'][$playerstat_week] = $playerstat_completions;
            $freeagent[$player_playerid][$playerstat_week]['completionswk'] = $playerstat_completions;
            $freeagent[$player_playerid]['attempts'] += $playerstat_attempts;
            //$freeagent[$player_playerid]['attemptswk'][$playerstat_week] = $playerstat_attempts;
            $freeagent[$player_playerid][$playerstat_week]['attempts'] = $playerstat_attempts;
            $freeagent[$player_playerid]['passints'] += $playerstat_passints;
            //$freeagent[$player_playerid]['passintswk'][$playerstat_week] = $playerstat_passints;
            $freeagent[$player_playerid][$playerstat_week]['passints'] = $playerstat_passints;
            $freeagent[$player_playerid]['passyds'] += $playerstat_passyds;
            //$freeagent[$player_playerid]['passydswk'][$playerstat_week] = $playerstat_passyds;
            $freeagent[$player_playerid][$playerstat_week]['passyds'] = $playerstat_passyds;
            $freeagent[$player_playerid]['passtds'] += $playerstat_passtds;
            //$freeagent[$player_playerid]['passtdswk'][$playerstat_week] = $playerstat_passtds;
            $freeagent[$player_playerid][$playerstat_week]['passtds'] = $playerstat_passtds;
            $freeagent[$player_playerid]['recepts'] += $playerstat_recepts;
            //$freeagent[$player_playerid]['receptswk'][$playerstat_week] = $playerstat_recepts;
            $freeagent[$player_playerid][$playerstat_week]['recepts'] = $playerstat_recepts;
            $freeagent[$player_playerid]['recyds'] += $playerstat_recyds;
            //$freeagent[$player_playerid]['recydswk'][$playerstat_week] = $playerstat_recyds;
            $freeagent[$player_playerid][$playerstat_week]['recyds'] = $playerstat_recyds;
            $freeagent[$player_playerid]['rectds'] += $playerstat_rectds;
            //$freeagent[$player_playerid]['rectdswk'][$playerstat_week] = $playerstat_rectds;
            $freeagent[$player_playerid][$playerstat_week]['rectds'] = $playerstat_rectds;
        }

		$points = 0;
		
		foreach ($scoring as $stat => $statvalue)
		{
			$points += $freeagent[$player_playerid][$stat] * $statvalue;
		}
        
        if ($leagueid == 3 || $leagueid == 5)
        {
            $freeagent[$player_playerid]['points'] = floor($points);
        }
        else
        {
            $freeagent[$player_playerid]['points'] = round($points,2);
        }
        //if ($points = 0)
        //{
        //    unset($freeagent[$player_playerid]);
        //}
	}	
        //have to do a sort here to get top 25
    $sortedfreeagent = array_csort($freeagent,'points',SORT_DESC);

	$freeagentlength = (count($sortedfreeagent) > 20) ? 20 : count($sortedfreeagent);
	
    for ($count=1;$count<=$freeagentlength;$count++)
    {
        $currfreeagent = array_shift($sortedfreeagent);
        //print_r($currfreeagent);
    
		$form[$currfreeagent['playerid']]['playerid'] = array(
			'#type' => 'value',
			'#value' => $currfreeagent['playerid'],	
        );
		
		$form[$currfreeagent['playerid']]['lastname'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['lastname'],
		);
		
		$form[$currfreeagent['playerid']]['firstname'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['firstname'],
		);
		
		$form[$currfreeagent['playerid']]['pos'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['pos'],
		);
		
		$form[$currfreeagent['playerid']]['teamname'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['teamname'],
		);
		
		$form[$currfreeagent['playerid']]['year'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['year'],
		);
		
/*
		$form[$currfreeagent['playerid']]['xp2'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['xp2'],
		);
		
		$form[$currfreeagent['playerid']]['puntretyds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['puntretyds'],
		);
		
		$form[$currfreeagent['playerid']]['puntrettds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['puntrettds'],
		);
		
		$form[$currfreeagent['playerid']]['kickretyds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['kickretyds'],
		);
		
		$form[$currfreeagent['playerid']]['kickrettds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['kickrettds'],
		);
*/
		
		$form[$currfreeagent['playerid']]['carries'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['carries'],
		);
		
		$form[$currfreeagent['playerid']]['rushyds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['rushyds'],
		);
		
		$form[$currfreeagent['playerid']]['rushtds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['rushtds'],
		);
		
		$form[$currfreeagent['playerid']]['completions'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['completions'],
		);
		
		$form[$currfreeagent['playerid']]['attempts'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['attempts'],
		);
		
		$form[$currfreeagent['playerid']]['passints'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['passints'],
		);
		
		$form[$currfreeagent['playerid']]['passyds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['passyds'],
		);
		
		$form[$currfreeagent['playerid']]['passtds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['passtds'],
		);
		
		$form[$currfreeagent['playerid']]['recepts'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['recepts'],
		);
		
		$form[$currfreeagent['playerid']]['recyds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['recyds'],
		);
		
		$form[$currfreeagent['playerid']]['rectds'] = array(
			'#type' => 'markup',
			'#value' => $currfreeagent['rectds'],
		);
        
		$form[$currfreeagent['playerid']]['points'] = array(
			'#type' => 'markup',
            '#prefix' => '<div class="cluetip-title" title="Week 1: '.$currfreeagent['points'].'">',
            '#suffix' => '</div>',
			'#value' => $currfreeagent['points'],
		);
        
        //$form['addplayer'][$currfreeagent['playerid']] = array(
        //    '#type' => 'checkbox',
        //);

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
		
	//$form['next'] = array(
	//	'#type' => 'submit',
	//	'#value' => t('Submit Waiver Request'),
	//);
	
	$form['poschoice'] = array(
		'#type' => 'select',
		'#options' => $posoptions,
		'#multiple' => TRUE,
		'#title' => t('Positions'),
        '#default_value' => $form_state['storage']['stored_pos'],
		//'#size' => 1,
	);
	
	$form['namefilter'] = array(
		'#type' => 'textfield',
		'#title' => t('Last Name'),
		'#size' => 30,
        '#default_value' => $form_state['storage']['stored_name'],
	);
	
	$form['confchoice'] = array(
		'#type' => 'select',
		'#options' => $confoptions,
		'#multiple' => TRUE,
		'#title' => t('Conferences'),
        '#default_value' => $form_state['storage']['stored_conf'],
		//'#size' => 1,
	);
	
	$form['teamchoice'] = array(
		'#type' => 'select',
		'#options' => $teamoptions,
		'#title' => t('Team'),
        '#default_value' => $form_state['storage']['stored_team'],
	);
	
	$form['filter'] = array(
		'#type' => 'submit',
		'#value' => t('Apply filter'),
	);
	
    return $form;
}

function playersearch_form_validate($form, &$form_state)
{
/*
    if (!isset($leaguedb))
    {
        $leaguedb = connectdbs();
    }

	$starter_values = $form['starter'];
    $starters = array();
    
	foreach($starter_values as $key => $value)
	{
        //dpm($form[$key]['pos']['#value']);

		if (is_numeric($key))
        {
            if (!$value['#value'])
            {
                $starters[$form[$key]['pos']['#value']] += 1 ;
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

        if (!isset($starters[$posreq_pos]))
            $starters[$posreq_pos] = 0;
        
        $poscheck[$posreq_pos]['min'] += $posreq_minimum;
        $poscheck[$posreq_pos]['max'] += $posreq_maximum;

        if ($posreq_flexgroup && $posreq_pos != 'FLX')
        {
            
            $poscheck['FLX']['starters'] += $starters[$posreq_pos];
            if ($poscheck['FLX']['total'])
            {
                $poscheck['FLX']['total'] += $posreq_minimum;
            }
            else
            {
                $poscheck['FLX']['total'] = $posreq_maximum;
            }
            
            $poscheck['FLX']['min'] += $posreq_minimum;
        }
    }

    foreach ($poscheck as $poskey => $posvalue)
    {
        if (($starters[$poskey] < $posvalue['min'] || $starters[$poskey] > $posvalue['max']) && $poskey != 'FLX')
        {
            if ($posvalue['min'] == $posvalue['max'])
            {
                if ($posvalue['min'] == 1)
                {
                    $message = 'You submitted '.$starters[$poskey].' '.$poskey.'s. '.$posvalue['min'].' is required.';
                }
                else
                {
                    $message = 'You submitted '.$starters[$poskey].' '.$poskey.'. '.$posvalue['min'].' are required.';
                }
            }
            else
            {
                $message = 'You submitted '.$starters[$poskey].' '.$poskey.'. '.$posvalue['min'].'-'.$posvalue['max'].' are required.';
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
*/
}


function playersearch_form_submit($form, &$form_state)
{
	$clicked = $form_state['clicked_button']['#value'];
	
	switch ($clicked)
	{
		case 'Apply filter':
			$form_state['rebuild'] = TRUE;
			$form_state['storage'] = array('stored_pos' => $form_state['values']['poschoice'],'stored_name' => $form_state['values']['namefilter'],'stored_conf' => $form_state['values']['confchoice'],'stored_team' => $form_state['values']['teamchoice']);
			break;
	}
/*
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
            if ($value['#value'])
            {
                $starter = 'N';
            }
            else
            {
                $starter = 'Y';
            }

            $addlineupquery = "INSERT INTO playerlineups VALUES ({$form['week']['#value']}, $key, '$starter', 0, {$form['teamid']['#value']})";
            $addlineupresult = $leaguedb->query($addlineupquery);
            DBerror($addlineupquery, $addlineupresult);	
        }
    }
	
	drupal_set_message("Lineup change saved.");

	unset ($form_state['storage']);
*/
}
?>