<?PHP
function draftplayers_all()
{

	$draft = drupal_get_form('draftplayers_form');
	
	return $draft;
}

function draftplayers_form($form_state)
{
//    if (isset($form_state['storage']['page_two']))
//    {
//        return draftplayers_form_page_2($form_state);
//    }

?>
	<script type="text/javascript">

	function change_pick()
	{
		var baseurl = "http://www.ctgffl.com/lgmgmt/StartDraft.php?pick=";
		selectedPick = document.getElementById("predraftpick").value;
		if (selectedPick.length != 0)
		{
		  location.href = baseurl + selectedPick ;
		}

	}

	function confirmPick(dropdown)
	{
		var confirmbtn = confirm ("Press OK to select "+dropdown+" or cancel to reselect.")
		if (confirmbtn)
		{
			return true
		}
		else
		{
			return false
		}
	}

	function MoveItem(ctrlSource, ctrlTarget)
	{
		var Source = document.getElementById(ctrlSource);
		var Target = document.getElementById(ctrlTarget);

		if ((Source != null) && (Target != null))
		{
				var newOption = new Option(); // Create a new instance of ListItem
				newOption.text = Source.options[Source.options.selectedIndex].text;
				newOption.value = Source.options[Source.options.selectedIndex].value;

				Target.options[Target.length] = newOption; //Append the item in Target
				Source.remove(Source.options.selectedIndex);  //Remove the item from Source
		}
	}

	// -------------------------------------------------------------------
	// hasOptions(obj)
	//  Utility function to determine if a select object has an options array
	// -------------------------------------------------------------------
	function hasOptions(obj)
	{
		if (obj!=null && obj.options!=null) { return true; }
		return false;
	}

	// -------------------------------------------------------------------
	// swapOptions(select_object,option1,option2)
	//  Swap positions of two options in a select list
	// -------------------------------------------------------------------
	function swapOptions(obj,i,j)
	{
		var o = obj.options;
		var i_selected = o[i].selected;
		var j_selected = o[j].selected;
		var temp = new Option(o[i].text, o[i].value, o[i].defaultSelected, o[i].selected);
		var temp2= new Option(o[j].text, o[j].value, o[j].defaultSelected, o[j].selected);
		o[i] = temp2;
		o[j] = temp;
		o[i].selected = j_selected;
		o[j].selected = i_selected;
	}

	// -------------------------------------------------------------------
	// moveOptionUp(select_object)
	//  Move selected option in a select list up one
	// -------------------------------------------------------------------
	function moveOptionUp(obj)
	{
		if (!hasOptions(obj)) { return; }
		for (i=0; i<obj.options.length; i++)
		{
			if (obj.options[i].selected)
			{
				if (i != 0 && !obj.options[i-1].selected)
				{
					swapOptions(obj,i,i-1);
					obj.options[i-1].selected = true;
				}
			}
		}
	}

	// -------------------------------------------------------------------
	// moveOptionDown(select_object)
	//  Move selected option in a select list down one
	// -------------------------------------------------------------------
	function moveOptionDown(obj)
	{
		if (!hasOptions(obj)) { return; }
		for (i=obj.options.length-1; i>=0; i--)
		{
			if (obj.options[i].selected)
			{
				if (i != (obj.options.length-1) && ! obj.options[i+1].selected)
				{
					swapOptions(obj,i,i+1);
					obj.options[i+1].selected = true;
				}
			}
		}
	}


	function showSelected()
	{
		var optionList = document.getElementById("PreDrafts").options;
		var data = '';
		var len = optionList.length;
		for(i=0; i<len; i++)
		{
			data += optionList.item(i).value;
			data += ',';
		}
		alert(data);
	}

	function submitlist(alist)
	{
		// selects all values in list box
		for (i = 0; i < alist.length; ++i)
		{
			alist[i].selected = "true";
		}

		// changes the name of the <select> tag to match an array for PHP processing
		document.forms[0].PreDrafts.name = "PreDrafts[]";
	}
	</script>
<?PHP
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

	$currentpicktext = "The current pick ($roundpicknext) will expire at $pickduetime</i><br><br><br>";

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
	
	$form['draftroundpick'] = array(
		'#type' => 'hidden',
		'#value' => $roundpicknext,
	);
	
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Submit draft pick',
	);

	$form['#redirect'] = 'scoreos/draftboard';

	if ($picksleft != 0)
	{
		$form['submit']['#disabled'] = TRUE;
	}
/*
	if ($picksleft == 0 || $nextteampick < $nextpick)
	{
		$output .= '<form name="draftform" action="ConfirmDraft.php" onSubmit="return confirmPick(document.draftform.draftplayerlist0[document.draftform.draftplayerlist0.selectedIndex].text)" method="POST"><input type="hidden" name="confirm" value="1">';
	}
	else
	{
		$output .= '<form name="draftform" action="ConfirmDraft.php" method="POST"><tr><td align="center">Picks for: <select name="predraftpick" id="predraftpick" OnChange="change_pick()">';

		$teampicks = $leaguedb->queryCol($nextteampickquery);
		DBerror($nextteampickquery,$teampicks);
		foreach ($teampicks as $predraftpick)
		{
			$roundofpick = ceil($predraftpick/$numteams);
			$numberofpick = $predraftpick%$numteams;
			if ($numberofpick == 0) $numberofpick = $numteams;
			$numberofpick = sprintf("%02s", $numberofpick);
			$roundpick = $roundofpick.".".$numberofpick;
			$output .= "<option ";
			if ($nextteampick == $predraftpick)
				$output .= "SELECTED ";
			$output .= "value=\"".$predraftpick."\"";
			$output .= ">";
			$output .= $roundpick;
		}
		$output .= "</select></td></tr>";
	}
	//Load the predrafts
	$predraftpicksquery = "SELECT playerid FROM predraft WHERE leagueid = $leagueid AND teamid = $chosenteam AND pick = $nextteampick ORDER BY pickorder";
	$predraftpicks = $leaguedb->queryCol($predraftpicksquery );
	DBerror($predraftpicksquery, $predraftpicks);

	$output .= "<table><tr>";
	$output .= '<td>
			<p align="center">
				<input onclick="MoveItem(\'FreeAgents\', \'PreDrafts\');" type="button" value="Add player" />
			</p>
			<p align="center">
				<input onclick="MoveItem(\'PreDrafts\', \'FreeAgents\');" type="button" value="Remove player" />
			</p>
		   </td>';

	$output .= "<td align=\"center\">";
	$output .= "<br><select id=\"FreeAgents\" name=\"freeagents\">";
	foreach ($freeagents as $fa_playerid => $currFA)
	{
		//extract ($currFA, EXTR_PREFIX_ALL, "fa");
		$FreeAgentName = $fa_lastname.', '.$fa_firstname.' - '.$fa_pos.' - '.$fa_lgteamabbr.'('.$fa_year.')';
//        $output .= "<option Value=".$fa_playerid.">".$FreeAgentName."</option>";
		$output .= "<option Value=".$fa_playerid.">".getPlayerNameLastFirst($fa_playerid, $leaguedb)."</option>";
	}
	$output .= '</select></td></tr><tr>
			<td><br><br><br><br>
				<p align="center">
					<INPUT TYPE=button VALUE="Move Up" onClick="moveOptionUp(this.form[\'PreDrafts\'])">
				</p>
				<p align="center">
					<INPUT TYPE=button VALUE="Move Down" onClick="moveOptionDown(this.form[\'PreDrafts\'])">
				</p>
			</td>
			<td align="center">
				<select name="PreDrafts" id="PreDrafts" size=20 multiple width=100>';
	foreach ($predraftpicks as $pre_playerid)
	{
		$output .= "<option Value=".$pre_playerid.">".getPlayerNameLastFirst($pre_playerid, $leaguedb)."</option>";
	}

	$output .= '</select></td></tr></table><br>
	<tr><td align="center" colspan=2>
	<input type="submit" name="submittrans" value="Submit Picks" onClick="javascript:submitlist(document.forms[0].PreDrafts);">
	</td>
</tr>
	<input type="hidden" name="leagueid" value="'.$leagueid.'">
	<input type="hidden" name="chosenteam" value="'.$chosenteam.'">
	<input type="hidden" name="chosenpick" value="'.$nextteampick.'">
	<input type="hidden" name="nextpick" value="'.$nextpick.'">
	</form>
	</table></td></tr></table>';
*/
	return $form;
}

function draftplayers_form_submit($form, &$form_state)
{
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
//		$form_state['storage']['page_two'] = TRUE;
//		$form_state['storage']['teamid'] = $form_state['values']['team'];
	}
	else
	{
		extract ($form_state['values'], EXTR_PREFIX_ALL, "draft");
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

		$nextpick = $draft_draftpick + 1;
		$nextpickquery = "SELECT playerid FROM draftpicks WHERE pick=$nextpick";
		$nextpickplayer = $leaguedb->queryOne($nextpickquery);
		
		while ($nextpickplayer)
		{
			$nextpick++;
			$nextpickquery = "SELECT playerid FROM draftpicks WHERE pick=$nextpick";
			$nextpickplayer = $leaguedb->queryOne($nextpickquery);
		}

		$draftupdatequery = "UPDATE draft SET currentpick = $nextpick, pickdue = '$nextpickdue' WHERE leagueid = $draft_league";
		$draftupdate = $leaguedb->query($draftupdatequery);
		DBerror($draftupdatequery,$draftupdate);

		drupal_set_message($teamname." added ".$addplayername);

		emailPick($leaguedb, $draft_draftpick, $draft_addplayer);

		$form_state['redirect'] = 'scoreos/draftboard';
	        unset ($form_state['storage']);
	}
}
