<?PHP
function draftplayers_all()
{

	$draft = drupal_get_form('draftplayers_form');
	
	return $draft;
}

function theme_draftplayers_form($form) {
	//dpm($form);
	$output = '';
	drupal_add_tabledrag('draggable-table', 'order', 'sibling', 'weight-group');
	$header = array('Name', 'Weight');
	foreach (element_children($form['predraft']) as $key) {
		$element = &$form['predraft'][$key];
		$element['weight']['#attributes']['class'] = 'weight-group';
		$row = array();
		$row[] = drupal_render($element['name']);
//		$row[] = 'filler';
//		$row[] = l('Edit', 'node/'. $element['id']['#value'] . '/edit') . ' | ' .
//			l('Delete', 'node/'. $element['id']['#value'] . '/delete');
		$row[] = drupal_render($element['weight']) . drupal_render($element['playerid']);
		$rows[] = array('data' => $row, 'class' => 'draggable');
	}
	$output .= drupal_render($form);
	$output .= theme('table', $header, $rows, array('id' => 'draggable-table'));
	return $output;
}

function draftplayers_form($form_state)
{
	global $user;
	$imgpath = "http://www.ncaaproject.com/".drupal_get_path('module','scoreos')."/images";
	
	//drupal_add_js('script.js');
	
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

	$teamid = arg(2);
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

	$teamnamequery = "SELECT teamfirstname as teamname FROM teams WHERE teamid = $teamid and leagueid = 6";
	$teamname = $leaguedb->queryOne($teamnamequery);
	DBerror($teamnamequery,$teamname);

	$chosenteam = $teamid;

	$nextteampickquery = "SELECT pick FROM draftpicks WHERE playerid IS NULL AND leagueid = $leagueid AND teamid = $chosenteam ORDER BY pick";
	$nextteampick = $leaguedb->queryOne($nextteampickquery );
	DBerror($nextteampickquery, $nextteampick);

	if ($_GET['pick'])
		$nextteampick = $_GET['pick'];

	$nextpickquery = "SELECT currentpick, pickdue, numteams FROM draft WHERE leagueid = $leagueid";
	$nextpickresult = $leaguedb->queryRow($nextpickquery );
	DBerror($nextpickquery, $nextpickresult);

	$nextpick = $nextpickresult['currentpick'];
	$pickduetime = $nextpickresult['pickdue'];
	$numteams = $nextpickresult['numteams'];

	$roundofnextpick = ceil($nextpick/$numteams);
	$numberofnextpick = $nextpick%$numteams;
	if ($numberofnextpick == 0) $numberofnextpick = $numteams;
	$numberofnextpick = sprintf("%02s", $numberofnextpick);
	$roundpicknext = $roundofnextpick.".".$numberofnextpick;

	$picksleftquery = "SELECT COUNT(teamid) FROM draftpicks WHERE leagueid = $leagueid AND playerid IS NULL AND pick >= $nextpick AND pick < $nextteampick";
	$picksleft = $leaguedb->queryOne($picksleftquery );
	DBerror($picksleftquery, $picksleft);

	//load roster

	$proquery = "SELECT includepro FROM leagues WHERE leagueid = $leagueid";
	$includepro = $leaguedb->queryOne($proquery);
	DBerror($proquery, $includepro);

	$rosterquery = "SELECT distinct players.playerid as playerid, lastname, firstname, lgteamabbr, year, players.pos as playerpos FROM playerteams, players, positionorder, lgteams WHERE playerteams.teamid = $chosenteam AND playerteams.playerid = players.playerid AND players.pos = positionorder.pos AND players.lgteamid = lgteams.lgteamid ORDER BY posorder, lastname";
	$rosterresult = $leaguedb->query($rosterquery);
	DBerror($rosterquery,$rosterresult);

	$roster = "<ul class='roster draft'>";
	while ($rosterrow = $rosterresult->fetchrow(DB_FETCHMODE_ASSOC))
	{
		extract($rosterrow, EXTR_PREFIX_ALL, "ros");
		switch ($ros_year)
		{
			case 'Freshman':
				$ros_year='FR';
				break;
			case 'Sophomore':
				$ros_year='SO';
				break;
			case 'Junior':
				$ros_year='JR';
				break;
			case 'Senior':
				$ros_year='SR';
				break;
		}
	$roster .= "<li>$ros_playerpos $ros_firstname $ros_lastname ($ros_year) - $ros_lgteamabbr</li>";
	}

	if ($includepro)
	{
		$rosterproquery = "SELECT distinct nflplayers.playerid as playerid, lastname, firstname, lgteamabbr, nflplayers.pos as playerpos FROM playerteams, nflplayers, positionorder, lgteams WHERE playerteams.teamid = $chosenteam AND playerteams.playerid = nflplayers.playerid AND nflplayers.pos = positionorder.pos AND nflplayers.lgteamid = lgteams.lgteamid ORDER BY posorder, lastname";
		$rosterproresult = $leaguedb->query($rosterproquery);
		if(PEAR::isError($rosterproresult)) 
		{
			var_export($form_state,TRUE);
			die('Failed to issue query, error message : ' . $rosterproresult->getMessage()." on ".$rosterproquery);
		}

		while ($rosterprorow = $rosterproresult->fetchrow(MDB2_FETCHMODE_ASSOC))
		{
			extract ($rosterprorow, EXTR_PREFIX_ALL, "ros");
			$teamquery = "SELECT lgteamname FROM lgteams WHERE lgteamid = $ros_lgteamid";
			$lgteamname = $leaguedb->queryOne($teamquery);
			$roster .= "<li>$ros_playerpos $ros_firstname $ros_lastname - $ros_lgteamabbr</li>";
		}
	}
	$roster .= "</ul>";

	//Load the free agents

	$proquery = "SELECT includepro FROM leagues WHERE leagueid = $leagueid";
	$includepro = $leaguedb->queryOne($proquery);
	DBerror($proquery, $includepro);

	$freeagentsquery = "SELECT DISTINCT playerid, firstname, lastname, lgteamid, pos, year FROM players WHERE (NOT EXISTS (SELECT playerid FROM playerteams WHERE leagueid = $leagueid AND players.playerid = playerteams.playerid) AND (pos = 'QB' OR pos = 'RB' OR pos = 'WR' OR pos = 'TE' OR pos = 'K' or pos = 'D')) ORDER BY lastname, firstname";
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

	if ($picksleft == 1)
	{
		$yourturn = "You are up next!";
	}
	elseif ($picksleft == 0)
	{
		$yourturn = "You are up!!";
	}
	elseif ($nextteampick < $nextpick)
	{
		$yourturn = "You have been skipped! Pick now!";
	}
	else
	{
		$yourturn = "You have ".$picksleft." picks before you";
	}

	$currentpicktext = "The current pick ($roundpicknext) will expire at $pickduetime</i><br><br>";

	$form['teamname'] = array(
		'#prefix' => '<h2 class="teamname draft">',
		'#suffix' => '</h2>',
		'#value' => $teamname,
	);

	$form['yourteam'] = array(
		'#value' => $roster,
	);

	$form['yourturn'] = array(
		'#prefix' => '<h4 class="yourturn draft">',
		'#suffix' => '</h4>',
		'#value' => $yourturn,
	);

	$form['currentpicktext'] = array(
		'#prefix' => '<span class="currentpicktext draft">',
		'#suffix' => '</span>',
		'#value' => $currentpicktext,
	);

	$form['addplayer'] = array(
		'#type' => 'select',
		'#title' => t('Player to draft'),
		'#options' => $freeagents,
	);

	$form['draftteam'] = array(
		'#type' => 'hidden',
		'#value' => $chosenteam,
	);
	
	$form['league'] = array(
		'#type' => 'hidden',
		'#value' => $leagueid,
	);
	
	$form['draftround'] = array(
		'#type' => 'hidden',
		'#value' => $roundofnextpick,
	);
	
	$form['draftpick'] = array(
		'#type' => 'hidden',
		'#value' => $nextpick,
	);
	
	$form['draftteampick'] = array(
		'#type' => 'hidden',
		'#value' => $nextteampick,
	);
	
	$form['draftroundpick'] = array(
		'#type' => 'hidden',
		'#value' => $roundpicknext,
	);
	
	if ($picksleft != 0)
	{
		$data = array();
		$predraftquery = "SELECT playerid, pickorder FROM predraft WHERE leagueid=6 AND teamid = $teamid AND pick = $nextteampick";
		$predraftresult = $leaguedb->query($predraftquery);
		while ($row = $predraftresult->fetchrow(MDB2_FETCHMODE_ASSOC)) {
			$row['playername'] = getPlayerNameLastFirst($row['playerid'], $leaguedb);
			$data[] = array(
				'playerid' => $row['playerid'],
				'name' => $row['playername'],
				'weight' => $row['pickorder'],
			);
		}
  
		$form['predraft'] = array();
		$form['predraft']['#tree'] = TRUE;
  
		foreach ($data as $item) {
			$form['predraft'][$item['playerid']] = array(
				'name' => array(
					'#type' => 'markup',
					'#value' => $item['name'],
				),
				'weight' => array(
					'#type' => 'weight',
					'#delta' => count($data),
					'#default_value' => isset($item['weight']) ? $item['weight'] : 0,
				),
				'playerid' => array(
					'#type' => 'hidden',
					'#value' => $item['playerid'],
				),
			);
		}
	}

	if ($picksleft != 0)
	{
		$form['save']['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Save Predraft',
		);

		$form['add']['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Add player to predraft',
		);

		$form['clear']['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Clear predrafts',
		);
	}
	else
	{
		$form['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Submit draft pick',
	);
	}
	//$form['#redirect'] = 'scoreos/draftboard';

	return $form;
}

function draftplayers_form_submit($form, &$form_state)
{
	//dpr($form_state);
	extract ($form_state['values'], EXTR_PREFIX_ALL, "draft");

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
	   
	if ($form_state['clicked_button']['#value'] == 'Save Predraft') {
		$clearpredraftquery = "DELETE FROM predraft WHERE leagueid = 6 AND teamid = $draft_draftteam AND pick = $draft_draftteampick";
		$clearpredraftresult = $leaguedb->query($clearpredraftquery);
		DBerror($clearpredraftquery,$clearpredraftresult);
		foreach ($form_state['values']['predraft'] as $item) {
			$updatepredraftquery = "INSERT INTO predraft VALUE (6, $draft_draftteam, $draft_draftteampick, ".$item['weight'].", ".$item['playerid'].")";
			$updatepredraftresult = $leaguedb->query($updatepredraftquery);
			DBerror($updatepredraftquery,$updatepredraftresult);
		}
		$form_state['redirect'] = 'scoreos/draftplayers';
	}
	elseif ($form_state['clicked_button']['#value'] == 'Add player to predraft') {
		$newweight = count($form_state['values']['predraft']) + 1;
		$addtopredraftquery = "INSERT INTO predraft VALUE (6, $draft_draftteam, $draft_draftteampick, $newweight, $draft_addplayer)";
		$addtopredraftresult = $leaguedb->query($addtopredraftquery);
		DBerror($addtopredraftquery,$addtopredraftresult);
		$form_state['redirect'] = 'scoreos/draftplayers';
	}
	elseif ($form_state['clicked_button']['#value'] == 'Clear predrafts') {
		$clearpredraftquery = "DELETE FROM predraft WHERE leagueid = 6 AND teamid = $draft_draftteam AND pick = $draft_draftteampick";
		$clearpredraftresult = $leaguedb->query($clearpredraftquery);
		DBerror($clearpredraftquery,$clearpredraftresult);
	}
	else
	{
		$addplayername = getPlayerName($draft_addplayer, $leaguedb);
		
		$teamnamequery = "SELECT teamfirstname FROM teams WHERE teamid=$draft_draftteam";
		$teamname = $leaguedb->queryOne($teamnamequery);

		$draftaddquery = "INSERT INTO playerteams VALUES($draft_addplayer, $draft_league, $draft_draftteam, NULL)";
		$draftadd = $leaguedb->query($draftaddquery);
		DBerror($draftaddquery,$draftadd);
	       
		$draftpicksquery = "UPDATE draftpicks SET playerid = $draft_addplayer, picktime = NOW() WHERE teamid = $draft_draftteam AND pick = $draft_draftpick";
		$draftpicks = $leaguedb->query($draftpicksquery);
		DBerror($draftpicksquery,$draftpicks);
	       
		$dvupdatequery = "INSERT INTO ctgffldv VALUES($draft_addplayer, $draft_draftround)";
		$dvupdate = $leaguedb->query($dvupdatequery);
		DBerror($dvupdatequery,$dvupdate);
		
		$now = date('m-d-Y H:i:s');
		$now = date_parse($now);
		if ($now['hour'] > 16)
		{
			$nextpick['hour'] = $now['hour'] - 8;
			if ($nextpick['hour'] < 10)
				$nextpick['hour'] = '0'.$nextpick['hour'];
			$nextpick['day'] = $now['day'] + 1;
			if ($nextpick['day'] < 10)
				$nextpick['day'] = '0'.$nextpick['day'];
		}
		else
		{
			$nextpick['hour'] = $now['hour'] + 8;
			$nextpick['day'] = $now['day'];
		}

		$now['month'] = '0'.$now['month'];
		$nextpickdue = $now['year']."-".$now['month']."-".$nextpick['day']." ".$nextpick['hour'].":".$now['minute'].":".$now['second'];

		$pickmsg[] = $teamname." added ".$addplayername;
		
		$nextpick = $draft_draftpick + 1;

		unset($playerchosen);
		$playerkept=1; //gets us through the while the first time

		while ($playerchosen || $playerkept)
		{
			$nextpickquery = "SELECT teamid, playerid FROM draftpicks WHERE pick=$nextpick";
			$nextpickplayer = $leaguedb->queryRow($nextpickquery);
			if ($playerkept = $nextpickplayer['playerid'])
			{
				$teamnamequery = "SELECT teamfirstname FROM teams WHERE teamid=".$nextpickplayer['teamid'];
				$teamname = $leaguedb->queryOne($teamnamequery);
		
				$pickmsg[] = $teamname.' kept '.getPlayerName($playerkept,$leaguedb);
				$nextpick++;
			}
			else
			{
				unset($playerchosen);
				$predraftquery = "SELECT playerid FROM predraft WHERE leagueid = 6 AND pick = $nextpick AND teamid = ".$nextpickplayer['teamid']." ORDER BY pickorder ASC";
				$predraftresult = $leaguedb->query($predraftquery);
			
				while (($predraftrow = $predraftresult->fetchrow(MDB2_FETCHMODE_ASSOC)) && !$playerchosen)
				{
					$playertakenquery = "SELECT teamid FROM playerteams WHERE leagueid = 6 AND playerid = ".$predraftrow['playerid'];
					$playertaken = $leaguedb->queryOne($playertakenquery);
					if (!$playertaken)
					{
						$playerchosen = $predraftrow['playerid'];

						$teamnamequery = "SELECT teamfirstname FROM teams WHERE teamid=".$nextpickplayer['teamid'];
						$teamname = $leaguedb->queryOne($teamnamequery);

						$pickmsg[] = $teamname.' predrafted '.getPlayerName($playerchosen,$leaguedb);

						$draftaddquery = "INSERT INTO playerteams VALUES($playerchosen, $draft_league, ".$nextpickplayer['teamid'].", NULL)";
						$draftadd = $leaguedb->query($draftaddquery);
						DBerror($draftaddquery,$draftadd);
					       
						$draftpicksquery = "UPDATE draftpicks SET playerid = $playerchosen, picktime = NOW() WHERE teamid = {$nextpickplayer['teamid']} AND pick = $nextpick";
						$draftpicks = $leaguedb->query($draftpicksquery);
						DBerror($draftpicksquery,$draftpicks);
					       
						$roundofpick = ceil($nextpick/14);
						$dvupdatequery = "INSERT INTO ctgffldv VALUES($playerchosen, $roundofpick)";
						$dvupdate = $leaguedb->query($dvupdatequery);
						DBerror($dvupdatequery,$dvupdate);

						$nextpick++;
					}
				}
			}
		}

		$draftupdatequery = "UPDATE draft SET currentpick = $nextpick, pickdue = '$nextpickdue' WHERE leagueid = $draft_league";
		$draftupdate = $leaguedb->query($draftupdatequery);
		DBerror($draftupdatequery,$draftupdate);

		foreach ($pickmsg as $currmsg)
		{
			drupal_set_message($currmsg);
		}

		reset($pickmsg);
		array_shift($pickmsg);
		emailPick($leaguedb, $draft_draftpick, $draft_addplayer, $pickmsg);

		$form_state['redirect'] = 'scoreos/draftboard';
	        unset ($form_state['storage']);
	}
}
