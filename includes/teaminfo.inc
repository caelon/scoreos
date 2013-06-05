<?php
function teaminfo_all()
{
	global $user;
	$imgpath = "http://www.ncaaproject.com/".drupal_get_path('module','scoreos')."/images";
	
	jq_add('cluetip');
	
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
	$leagueidquery = "SELECT * FROM leagues WHERE leagueurl = '$leagueurl'";
	$leagueidrow = $leaguedb->queryRow($leagueidquery);
	DBerror($leagueidquery, $leagueidrow);
    
    $leagueid = $leagueidrow['leagueid'];
    $includepro = $leagueidrow['includepro'];
	$includedv = $leagueidrow['includedv'];
    $homefieldadv = $leagueidrow['homefieldadv'];

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

	if (!isset($teamid))
	{
		$teamlistquery = "SELECT teamid, teamfirstname, primcolor, seccolor FROM teams WHERE leagueid = $leagueid AND active = 1 ORDER by teamfirstname";
		$teamlistresult = $leaguedb->query($teamlistquery);
		DBerror($teamlistquery,$teamlistresult);
		$teaminfocontent = "<table><tr>";
		$columnnum = 1;
		while ($teamlistrow = $teamlistresult->fetchrow())
		{
			extract ($teamlistrow, EXTR_PREFIX_ALL, "teamlist");
			if ($columnnum > 5)
			{
				$columnnum = 1;
				$teaminfocontent .= "</tr><tr>";
			}
			$teaminfocontent .= "<td bgcolor=\"#$teamlist_primcolor\" width=\"20%\"><a href=\"http://$leagueurl/scoreos/teaminfo/$teamlist_teamid\"><font color=\"#$teamlist_seccolor\">$teamlist_teamfirstname</font></a></td>";
			$columnnum++;
		}
		$teaminfocontent .= "</tr></table>";
	}
	else
	{
		//$teamid = 5;
		$teaminfoquery = "SELECT * FROM teams WHERE teamid = $teamid";
		$teaminforow = $leaguedb->queryRow($teaminfoquery);
		DBerror($teaminfoquery,$teaminforow);
		//print_r($teaminforow);
		extract ($teaminforow, EXTR_PREFIX_ALL, "team");

		//$topcollidquery = "SELECT players.playerid as topid, SUM(points) as totalpoints FROM players, playerlineups WHERE playerlineups.playerid=players.playerid AND playerlineups.teamid = $teamid AND playerlineups.starter = 1 AND players.year != 'Pro' GROUP BY topid ORDER BY totalpoints DESC";
		$topcollidquery = "SELECT players.playerid as topid, dv FROM players, playerteams, ctgffldv WHERE playerteams.teamid = $teamid AND players.year != 'Pro' and players.playerid = ctgffldv.playerid and playerteams.playerid = players.playerid ORDER BY dv";
		$topcollidrow = $leaguedb->queryRow($topcollidquery);
		DBerror($topcollidquery,$topcollidrow);
		extract ($topcollidrow, EXTR_PREFIX_ALL, "topcoll");

		$collnameline = getPlayerName($topcoll_topid, $leaguedb);
		$topcoll_picture = $topcoll_topid.".png";

        if ($includepro)
        {
            //find top pro player
//           $topproidquery = "SELECT nflplayers.playerid as topid, SUM(points) as totalpoints FROM nflplayers, playerlineups WHERE playerlineups.playerid=nflplayers.playerid AND playerlineups.teamid = $teamid AND playerlineups.starter = 1 AND nflplayers.year = 'Pro' GROUP BY topid ORDER BY totalpoints DESC";
            $topproidquery = "SELECT nflplayers.playerid as topid, dv FROM nflplayers, playerteams, ctgffldv WHERE playerteams.teamid = $teamid AND nflplayers.year = 'Pro' and nflplayers.playerid = ctgffldv.playerid and playerteams.playerid = nflplayers.playerid ORDER BY dv";
            $topproidrow = $leaguedb->queryRow($topproidquery);
            DBerror($topproidquery,$topproidrow);
            $toppro_id = $topproidrow['topid'];

            $pronameline = getNFLPlayerName($toppro_id, $leaguedb);
            $toppro_picture = $toppro_id.".png";

            $teaminfocontent = "<table align=\"center\" border=\"0\"><tr><TD align=\"center\" width=\"40%\"><table><tr><td align=\"center\"><img src=\"$imgpath/players/$topcoll_picture\" width=\"100\" height=\"150\"><BR><font size=\"2\">$collnameline</font></td><td align=\"center\"><img src=\"$imgpath/players/$toppro_picture\" width=\"100\" height=\"150\"><BR><font size=\"2\">$pronameline </font></td></tr></table></TD><TD align=\"center\"></tD>";
        }
        else
        {
            $teaminfocontent = "<table align=\"center\" border=\"0\"><tr><TD align=\"center\" width=\"40%\"><img src=\"$imgpath/players/$topcoll_picture\" width=\"100\" height=\"150\"><BR><font size=\"2\">$collnameline</font></td>";
        }

		$teamuserarray = array('uid' => $team_teamownerid);
		$teamuser = user_load($teamuserarray);
		$profile =  profile_load_profile($teamuser);
		$owner_name = $teamuser->profile_first_name;
		$owner_name .= " ".$teamuser->profile_last_name;
		$owner_email = $teamuser->mail;
		$coach = strtoupper($owner_name);
	//	$owner_email = $user->mail;
		$viswinsquery = "SELECT COUNT(leagueid) FROM schedulehist WHERE visitorid = $teamid AND visitorscore > homescore";
		$viswins = $leaguedb->queryOne($viswinsquery);
		$homewinsquery = "SELECT COUNT(leagueid) FROM schedulehist WHERE homeid = $teamid AND visitorscore < homescore";
		$homewins = $leaguedb->queryOne($homewinsquery);
		$vislossesquery = "SELECT COUNT(leagueid) FROM schedulehist WHERE visitorid = $teamid AND visitorscore < homescore";
		$vislosses = $leaguedb->queryOne($vislossesquery);
		$homelossesquery = "SELECT COUNT(leagueid) FROM schedulehist WHERE homeid = $teamid AND visitorscore > homescore";
		$homelosses = $leaguedb->queryOne($homelossesquery);
		$totwins = $viswins + $homewins;
		$totlosses = $vislosses + $homelosses;
		$totpct = (($totwins+$totlosses)==0)?0:$totwins/($totwins+$totlosses);

	//	$teaminfocontent .= "<TD valign=\"top\" align = \"center\" width=\"300\"><font size=3 color=\"#$team_primcolor\">$team_teamname</font><BR>Coach: $coach<BR><font size=\"2\">$owner_user_from</font><BR><font size=\"1\"><a href=\"mailto:$owner_email\">email:$owner_email</a></font></TD><TD align=\"center\"><img src=\"http://www.ctgffl.com/images/teamlogos/$team_largelogo.png\" height=150></TD></TR><TR>
		$teaminfocontent .= "<TD valign=\"top\" align = \"center\" width=\"27%\"><font size=3 color=\"#$team_seccolor\">$team_teamfirstname</font><BR>Coach: $coach<BR><!--$owner_user_from-->$totwins-$totlosses (".sprintf("%01.3f", $totpct).")<BR><font size=\"1\"><a href=\"mailto:$owner_email\">email:$owner_email</a></font></TD><TD align=\"center\" width=\"33%\"><img src=\"$imgpath/teamlogos/$team_teamlogo.png\" height=150></TD></TR><TR>
		<TD align=\"center\" bgcolor=\"#000000\" width=\"49%\"><font color=\"#009900\">2012 SCHEDULE</font></TD>
		<TD align=\"center\" bgcolor=\"#000000\" width=\"1%\"><font color=\"#FFFFFF\">&nbsp;</font></TD>
		<TD align=\"center\" bgcolor=\"#000000\" width=\"1%\"><font color=\"#FFFFFF\">&nbsp;</font></TD>
		<TD align=\"center\" bgcolor=\"#000000\" width=\"49%\"><font color=\"#009900\">Roster</font></TD>
		</TR><TR><TD align=\"center\" valign=\"top\" colspan=2 width=\"50%\"><table><font size=1>";

		$schedulequery = "SELECT * FROM schedule WHERE visitorid = $teamid OR homeid=$teamid ORDER BY week";
		$scheduleresult = $leaguedb->query($schedulequery);
		DBerror($schedulequery,$scheduleresult);
		while ($schedulerow = $scheduleresult->fetchrow())
		{
			extract ($schedulerow, EXTR_PREFIX_ALL, "sched");
			$oppteamid = $sched_visitorid==$teamid ? $sched_homeid : $sched_visitorid;
			$oppquery = "SELECT teamfirstname FROM teams WHERE teamid = $oppteamid";
			$oppteamname = $leaguedb->queryOne($oppquery);
			$dayinyear = (($sched_week+39)*7);
			$wednesday = date("M d", mktime (12,0,0,0,$dayinyear-6,2012));
			$tuesday = date("M d", mktime (12,0,0,0,$dayinyear-1,2012));
			$saturday = date("M d", mktime (12,0,0,0,$dayinyear-4,2012));
			$teamcollscorequery = "SELECT SUM(a.points) as teampoints FROM playerlineups a, players b WHERE a.playerid = b.playerid AND starter = 1 AND week = $sched_week AND teamid = $teamid AND year !='Pro' GROUP BY teamid";
			$teamcollpoints = $leaguedb->queryOne($teamcollscorequery);
			$teamcollpoints = round($teamcollpoints,1);
			$oppteamcollscorequery = "SELECT SUM(a.points) as teampoints FROM playerlineups a, players b WHERE a.playerid=b.playerid AND starter = 1 AND week = $sched_week AND teamid = $oppteamid AND year !='Pro' GROUP BY teamid";
			$oppteamcollpoints = $leaguedb->queryOne($oppteamcollscorequery);
			$oppteamcollpoints = round($oppteamcollpoints,1);
			if ($teamid == $sched_visitorid)
            {
				$attext = "AT ";
                if ($homefieldadv)
                {
                    $oppteamcollpoints += $homefieldadv;
                }
            }
			else
            {
				$attext = "";
                if ($homefieldadv)
                {
                    $teamcollpoints += $homefieldadv;
                }
            }
			$collfontcolor = $teamcollpoints > $oppteamcollpoints ? "green" : "red";

			if ($includepro)
			{
				$teamproscorequery = "SELECT SUM(a.points) as teampoints FROM playerlineups a, nflplayers b WHERE a.playerid = b.playerid AND starter = 1 AND week = $sched_week AND teamid = $teamid AND year ='Pro' GROUP BY teamid";
				$teampropoints = $leaguedb->queryOne($teamproscorequery);
				$teampropoints = round($teampropoints,1);
				$oppteamproscorequery = "SELECT SUM(a.points) as teampoints FROM playerlineups a, nflplayers b WHERE a.playerid=b.playerid AND starter = 1 AND week = $sched_week AND teamid = $oppteamid AND year ='Pro' GROUP BY teamid";
				$oppteampropoints = $leaguedb->queryOne($oppteamproscorequery);
				$oppteampropoints = round($oppteampropoints,1);
				$profontcolor = $teampropoints > $oppteampropoints ? "green" : "red";
			}
			if ($sched_week == 0)
				$sched_week = -1; //allows the link to process Week 0 games

			$teaminfocontent .= "<TR><TD width=\"10%\">$saturday</TD><TD width=\"25%\">$attext<a href=\"http://$leagueurl/scoreos/teaminfo/$oppteamid\">$oppteamname</a></TD><TD align=\"center\" width=\"15%\"><a href=\"http://$leagueurl/scoreos/scorereport?chosenleague=$team_leagueid&chosenweek=$sched_week#$oppteamname\"><font color=\"$collfontcolor\">$teamcollpoints - $oppteamcollpoints</font>";
			if ($includepro)
				$teaminfocontent .= "<br><font color=\"$profontcolor\">$teampropoints - $oppteampropoints</font>";
			$teaminfocontent .= "</a></TD></TR>";
		}

		$teaminfocontent .= "</font></table><td width=\"1%\"><!--Divisional Record -->&nbsp;</td><td valign=\"top\"  width=\"49%\"><table>";
		
		//load records

			 //show college players
		if ($includedv)
		{
			$dvquery = "AND players.playerid = ctgffldv.playerid";
			$dvproquery = "AND nflplayers.playerid = ctgffldv.playerid";
			$dvcolumn = ", dv";
			$dvtable = ", ctgffldv";
		}
		else
		{
			$dvquery = "";
			$dvcolumn = "";
			$dvtable = "";
		}
		$rosterquery = "SELECT players.playerid as playerid, lastname, firstname, lgteamabbr, year, players.pos as playerpos, irweek $dvcolumn FROM playerteams, players, positionorder, lgteams $dvtable WHERE playerteams.teamid = $teamid AND playerteams.playerid = players.playerid AND players.pos = positionorder.pos AND positionorder.leagueid = $leagueid AND players.lgteamid = lgteams.lgteamid AND year != 'Pro' $dvquery ORDER BY posorder, lastname";
//		$rosterquery = "SELECT players.playerid as playerid, lastname, firstname, lgteamabbr, year, players.pos as playerpos, irweek FROM playerteams, players, positionorder, lgteams WHERE playerteams.teamid = $teamid AND playerteams.playerid = players.playerid AND players.pos = positionorder.pos AND positionorder.leagueid = $leagueid AND players.lgteamid = lgteams.lgteamid AND year != 'Pro' ORDER BY posorder, lastname";
		$rosterresult = $leaguedb->query($rosterquery);
		DBerror($rosterquery,$rosterresult);
		while ($rosterrow = $rosterresult->fetchrow())
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
			$pointsquery = "SELECT week, starter, points FROM playerlineups WHERE playerid = $ros_playerid and teamid = $team_teamid ORDER BY week";
			$pointsresult = $leaguedb->query($pointsquery);
			DBerror($pointsquery,$pointsresult);
			$teaminfocontent .= "<tr><td><font size=1>";
			if ($ros_irweek != NULL) $teaminfocontent .= "<del><font color=\"red\">";
			$teaminfocontent .= $ros_playerpos;
			if ($ros_irweek != NULL) $teaminfocontent .= "</font></del>";
			$teaminfocontent .= "</font></td><td>";
			$popup = "<div class=\"cluetip-title\" title=\"";
			while ($pointsrow = $pointsresult->fetchrow())
			{
				extract ($pointsrow, EXTR_PREFIX_ALL, "pts");
				//$popup .= "<ul><li>Week $pts_week: </li>";
				$popup .= "Week $pts_week: ";
				if ($pts_starter=="Y")
				{
					$popup .= "$pts_points";
				}
				else
				{
					$popup .= "$pts_points";
				}
			}
			$teaminfocontent .= "<font size=1>";
			if ($ros_irweek != NULL) $teaminfocontent .= "<del><font color=\"red\">";
			//if ($ros_irweek == NULL) $teaminfocontent .= "<a class=\"cluetip-title\" href=\"#\" title=\"Test|Test\">";
			if ($ros_irweek == NULL) $teaminfocontent .= $popup."\">";
            $teaminfocontent .= " $ros_firstname $ros_lastname ($ros_year) - $ros_lgteamabbr";
			if ($includedv) $teaminfocontent .= " ($ros_dv)";
			if ($ros_irweek == NULL) $teaminfocontent .= "</div>";
			//$teaminfocontent .= "</a>$popup";
			if ($ros_irweek != NULL) $teaminfocontent .= "</font></del>";
			$teaminfocontent .= "</font></div></td></tr>";
		}
		$teaminfocontent .= "<tr><td colspan=2>&nbsp;</td></tr>";
		
		if ($includepro)
		{
			$teaminfocontent .= "<tr><td colspan=2 align=\"center\"><em>Pro</em></td></tr>";
			 //show pro players
			$rosterquery = "SELECT DISTINCT nflplayers.playerid as playerid, lastname, firstname, lgteamabbr, year, nflplayers.pos as playerpos, irweek $dvcolumn FROM playerteams, nflplayers, positionorder, lgteams $dvtable WHERE playerteams.teamid = $teamid AND playerteams.playerid = nflplayers.playerid AND nflplayers.pos = positionorder.pos AND nflplayers.lgteamid = lgteams.lgteamid AND year = 'Pro' AND positionorder.leagueid = $leagueid $dvproquery ORDER BY posorder, lastname";
			$rosterresult = $leaguedb->query($rosterquery);
			DBerror($rosterquery,$rosterresult);
			while ($rosterrow = $rosterresult->fetchrow())
			{
				extract($rosterrow, EXTR_PREFIX_ALL, "ros");
				$pointsquery = "SELECT week, starter, points FROM playerlineups WHERE playerid = $ros_playerid and teamid = $team_teamid ORDER BY week";
				$pointsresult = $leaguedb->query($pointsquery);
				DBerror($pointsquery,$pointsresult);
				$teaminfocontent .= "<tr><td><font size=1>";
				if ($ros_irweek != NULL) $teaminfocontent .= "<del><font color=\"red\">";
				$teaminfocontent .= $ros_playerpos;
				if ($ros_irweek != NULL) $teaminfocontent .= "</font></del>";
				//$teaminfocontent .= "</font></td><td><A HREF=\"#\"  style=\"text-decoration:none\" ONMOUSEOVER=\"popup('<font color=#002064 face=arial size=2>";
				$teaminfocontent .= "</font></td><td>";
                /*
				$teaminfocontent .= "$ros_firstname $ros_lastname<br>";
				while ($pointsrow = $pointsresult->fetchrow())
				{
					extract ($pointsrow, EXTR_PREFIX_ALL, "pts");
					$teaminfocontent .=  "Week $pts_week: ";
					if ($pts_starter=="Y")
					{
						$teaminfocontent .= "<b>$pts_points</b><br>";
					}
					else
					{
						$teaminfocontent .= "$pts_points<br>";
					}
				}
				$teaminfocontent .= "</font>','#CCCCCC')\" ONMOUSEOUT=\"removeBox()\"><font size=1>";
                                           */
				if ($ros_irweek != NULL) $teaminfocontent .= "<del>";
				$teaminfocontent .= "<a class=\"playerpoints\" href=\"#\" title=\"Test\"> $ros_firstname $ros_lastname - $ros_lgteamabbr";
				if ($includedv)
				{
					$teaminfocontent .= " (".$ros_dv.")";
				}
				$teaminfocontent .= "</a>";
				if ($ros_irweek != NULL) $teaminfocontent .= "</del>";
				$teaminfocontent .= "</font></a></td></tr>";
			}
		}
		$teaminfocontent .= "</table></td></tr></table>";
	}
	return $teaminfocontent;
}
?>
