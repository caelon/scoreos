<?php
function scorereport_all()
{
	jq_add('cluetip');
	//drupal_set_message('Scores are unofficial until this message disappears.');
    $leaguedb = connectdbs();

	$league_includepro = 0;
	
    $leagueurl = $_SERVER['SERVER_NAME'];
    if (substr($leagueurl,0,4) == "www.")
    {
        $leagueurl = substr($leagueurl,4);
    }
    if ($leagueurl == "open-stats.com")
    {
        $leagueurl = "ncaaproject.com";
    }
	$leaguequery = "SELECT leagueid, homefieldadv, lgteaswr, tiebreaker, includepro FROM leagues WHERE leagueurl = '$leagueurl'";
	$leaguerow = $leaguedb->queryRow($leaguequery);
	DBerror($leaguequery, $leaguerow);

	extract ($leaguerow, EXTR_PREFIX_ALL, "league");
	$leagueid = $league_leagueid;
	
	unset($week);
    $week = 16; //set for bowl weeks
	if (arg(2))
	{
		$week = arg(2);
	}

	if (isset($week))
	{
		$gameweek = getLeagueWeek($week);
	}
	else
	{
		$gameweek = getLeagueWeek();
	}

    //Load the schedule
    if (date('N') < 2 && !isset($week))
    {
        $gameweek['week'] = $gameweek['week'] - 1;
    }
    $SchedQuery = "SELECT * FROM schedule WHERE week = {$gameweek['week']} AND leagueid = $leagueid";
    $SchedResult = $leaguedb->query($SchedQuery );
    DBerror($SchedQuery,$SchedResult);
    $SchedLastWeekQuery = "SELECT DISTINCT max(week) FROM schedule WHERE leagueid = $leagueid";
    $SchedLastWeek = $leaguedb->queryOne($SchedLastWeekQuery);
    DBerror($SchedLastWeekQuery,$SchedLastWeek);
    $scorereport_content = "<TABLE border=\"1\" width=\"700\">";
	$scorereport_content .= "<TR><td colspan=\"5\">";
	for ($listweek = 1; $listweek <= $SchedLastWeek; $listweek++)
	{
		if ($listweek == $gameweek['week'])
		{
			$scorereport_content .= "<strong>Week $listweek</strong> | ";
		}
		else
		{
			$scorereport_content .= "<a href=\"http://$leagueurl/scoreos/scorereport/$listweek\">Week $listweek</a> | ";
		}		
	}
	$scorereport_content .= "</td></TR><tr><td colspan=5 bgcolor=\"#000000\">&nbsp;</td></tr>";

    while ($ProcessSched = $SchedResult->fetchrow(MDB2_FETCHMODE_ASSOC))
    {
        $FirstTeam = $ProcessSched['visitorid'];
        $FirstTeamQuery = "SELECT teamfirstname, primcolor, seccolor FROM teams WHERE teamid = $FirstTeam";
        $FirstTeamResult = $leaguedb->query($FirstTeamQuery);
        DBerror($FirstTeamQuery,$FirstTeamResult);
        $FirstTeamRow = $FirstTeamResult->fetchrow(MDB2_FETCHMODE_ASSOC, 0);
        $FirstTeamName = $FirstTeamRow['teamfirstname'];
        $FirstTeamColor = "#".$FirstTeamRow['primcolor'];
        $FirstTeamFontColor = "#".$FirstTeamRow['seccolor'];
        $SecondTeam = $ProcessSched['homeid'];
        $SecondTeamQuery = "SELECT teamfirstname, primcolor, seccolor FROM teams WHERE teamid = $SecondTeam";
        $SecondTeamResult = $leaguedb->query($SecondTeamQuery);
        DBerror($SecondTeamQuery,$SecondTeamResult);
        $SecondTeamRow = $SecondTeamResult->fetchrow(MDB2_FETCHMODE_ASSOC, 0);
        $SecondTeamName = $SecondTeamRow['teamfirstname'];
        $SecondTeamColor = "#".$SecondTeamRow['primcolor'];
        $SecondTeamFontColor = "#".$SecondTeamRow['seccolor'];

//        $scorereport_content .= "<TR><a name=\"$FirstTeamName\"></a><a name=\"$SecondTeamName\"></a><TD colspan=\"2\" align=\"center\" bgcolor=\"$FirstTeamColor\" width=\"40%\"><a href=\"http://www.ctgffl.com/lgmgmt/teams.php?teamid=$FirstTeam\"><font color=\"$FirstTeamFontColor\">$FirstTeamName</font></a></TD><TD></TD><TD colspan=\"2\" align=\"center\" bgcolor=\"$SecondTeamColor\" width=\"40%\"><a href=\"http://www.ctgffl.com/lgmgmt/teams.php?teamid=$SecondTeam\"><font color=\"$SecondTeamFontColor\">$SecondTeamName</a></font></TD></TR>";
        $scorereport_content .= "<TR><a name=\"$FirstTeamName\"></a><a name=\"$SecondTeamName\"></a><TD colspan=\"2\" align=\"center\" bgcolor=\"$FirstTeamColor\" width=\"40%\"><a href=\"http://$leagueurl/scoreos/teaminfo/$FirstTeam\"><font color=\"$FirstTeamFontColor\">$FirstTeamName</font></a></TD><TD></TD><TD colspan=\"2\" align=\"center\" bgcolor=\"$SecondTeamColor\" width=\"40%\"><a href=\"http://$leagueurl/scoreos/teaminfo/$SecondTeam\"><font color=\"$SecondTeamFontColor\">$SecondTeamName</a></font></TD></TR>";
		if ($league_includepro)
		{
			$scorereport_content .=  "<TR><TD></TD><TD></TD><TD align=\"center\"><em>College</em></TD><TD></TD><TD></TD></TR>";
		}
        $FirstTeamCollStatsQuery = "SELECT players.playerid as playerid, lastname, firstname, lgteamabbr, players.pos, posorder, points, lgteams.lgteamid as lgteamid FROM players, positionorder, playerlineups, lgteams WHERE week={$gameweek['week']} AND playerlineups.playerid = players.playerid AND playerlineups.starter=1 AND playerlineups.teamid=$FirstTeam AND positionorder.pos = players.pos AND players.lgteamid = lgteams.lgteamid AND year !='Pro' AND positionorder.leagueid = $leagueid order by posorder, lastname";
        $FirstTeamCollStatsResult = $leaguedb->query($FirstTeamCollStatsQuery);
        DBerror($FirstTeamCollStatsQuery,$FirstTeamCollStatsResult);

        $FirstTeamCollStatsNumrows = $FirstTeamCollStatsResult->numrows();
    	$SecondTeamCollStatsQuery = "SELECT players.playerid as playerid, lastname, firstname, lgteamabbr, players.pos, posorder, points, lgteams.lgteamid as lgteamid FROM players, positionorder, playerlineups, lgteams WHERE week={$gameweek['week']} AND playerlineups.playerid = players.playerid AND playerlineups.starter=1 AND playerlineups.teamid=$SecondTeam AND positionorder.pos = players.pos AND players.lgteamid = lgteams.lgteamid AND year !='Pro' AND positionorder.leagueid = $leagueid order by posorder, lastname";
        $SecondTeamCollStatsResult = $leaguedb->query($SecondTeamCollStatsQuery );
        DBerror($SecondTeamCollStatsQuery,$SecondTeamCollStatsResult);
        $SecondTeamCollStatsNumrows = $SecondTeamCollStatsResult->numrows();
        if ($FirstTeamCollStatsNumrows == 0 && $SecondTeamCollStatsNumrows == 0)
        {
            $scorereport_content .=  "<TR><TD colspan=\"5\" align=\"center\">No score reported yet</TD></TR>";
        }
        else
        {
            $FirstTeamScore = 0;
            $SecondTeamScore = 0;
            $maxrows = max($FirstTeamCollStatsNumrows,$SecondTeamCollStatsNumrows);
            for ($i=0; $i<$maxrows; $i++)
            {
                $first_completions = 0;
                $first_attempts = 0;
                $first_passyds =0;
                $first_passtds=0;
                $first_passints=0;
                $first_carries=0;
                $first_rushyds=0;
                $first_rushtds=0;
                $first_recepts=0;
                $first_recyds=0;
                $first_rectds=0;
                $first_xp2=0;
                $first_kickreturntds=0;
                $first_puntreturntds=0;
                $first_pointsallow=0;
                $first_defwin = 0;
                $first_ydsallow = 0;
                $first_deftd=0;
                $first_specialteamtd=0;
                $first_safety=0;
                $first_sacks=0;
                $first_intercepts=0;
                $first_fumblerec=0;
                $first_fgmade=0;
                $first_patmade=0;
                $second_completions = 0;
                $second_attempts = 0;
                $second_passyds =0;
                $second_passtds=0;
                $second_passints=0;
                $second_carries=0;
                $second_rushyds=0;
                $second_rushtds=0;
                $second_recepts=0;
                $second_recyds=0;
                $second_rectds=0;
                $second_xp2=0;
                $second_kickreturntds=0;
                $second_puntreturntds=0;
                $second_pointsallow=0;
                $second_defwin = 0;
                $second_ydsallow = 0;
                $second_deftd=0;
                $second_specialteamtd=0;
                $second_safety=0;
                $second_sacks=0;
                $second_intercepts=0;
                $second_fumblerec=0;
                $second_fgmade=0;
                $second_patmade=0;
                if ($FirstTeamCollStatsNumrows)
                {
                    $FirstTeamCollRow = $FirstTeamCollStatsResult->fetchrow(MDB2_FETCHMODE_ASSOC, $i);
                    $FirstPlayerID = $FirstTeamCollRow['playerid'];
                    $FirstTeamColl = $FirstTeamCollRow['lgteamid'];
                    if ($FirstTeamCollRow['pos'] != 'D')
                    {
                        $FirstPlayer = $FirstTeamCollRow['firstname'].' '.$FirstTeamCollRow['lastname'].' - '.$FirstTeamCollRow['lgteamabbr'];
                        //$FirstPlayer = "<a href=\"http://www.ctgffl.com/lgmgmt/PlayerReport.php?player=$FirstPlayerID\">".$FirstPlayer."</a>";
                    }
                    else
                    {
                        $FirstPlayer = $FirstTeamCollRow['firstname'].' '.$FirstTeamCollRow['lastname'];
                    }

                    $FirstPlayerPoints = $FirstTeamCollRow['points'];
                    $firststatdetailquery = "SELECT * FROM playerstats WHERE playerid = $FirstPlayerID AND week = $gameweek";
                    $firststatdetail = $leaguedb->queryRow($firststatdetailquery);
                    if (is_array($firststatdetail))
                        extract ($firststatdetail, EXTR_PREFIX_ALL, "first");
                    
                    $position = $FirstTeamCollRow['pos'];
					if ($league_lgteaswr && $position == 'TE')
					{
						$position = 'WR';
					}
                }
                else
                {
                    $FirstPlayer = 'None selected';
                    $FirstPlayerPoints = '';
                }
    
                if ($SecondTeamCollStatsNumrows)
                {
                    $SecondTeamCollRow = $SecondTeamCollStatsResult->fetchrow(MDB2_FETCHMODE_ASSOC, $i);
                    $SecondPlayerID = $SecondTeamCollRow['playerid'];
                    if ($SecondTeamCollRow['pos'] != 'D')
                    {
                        $SecondPlayer = $SecondTeamCollRow['firstname'].' '.$SecondTeamCollRow['lastname'].' - 	'.$SecondTeamCollRow['lgteamabbr'];
                    }
                    else
                    {
                        $SecondPlayer = $SecondTeamCollRow['firstname'].' '.$SecondTeamCollRow['lastname'];
                    }

                    $SecondPlayerPoints = $SecondTeamCollRow['points'];
                    $secondstatdetailquery = "SELECT * FROM playerstats WHERE playerid = $SecondPlayerID AND week = $gameweek";
                    $secondstatdetail = $leaguedb->queryRow($secondstatdetailquery);
                    if (is_array($secondstatdetail))
                        extract ($secondstatdetail, EXTR_PREFIX_ALL, "second");

                    $position = $SecondTeamCollRow['pos'];
					if ($league_lgteaswr && $position == 'TE')
					{
						$position = 'WR';
					}
                }
                else
                {
                    $SecondPlayer = 'None selected';
                    $SecondPlayerPoints = '';
                }
                
    //			echo "<TR><TD width=\"38%\">$FirstPlayer</TD><TD width=\"7%\" align=\"center\">$FirstPlayerPoints</TD><TD width=\"10%\" align=\"center\">$position</TD><TD width=\"38%\">$SecondPlayer</TD><TD width=\"7%\" align=\"center\">$SecondPlayerPoints</TD></tr>";
    /* from here */
    if ($FirstTeamColl)
        $FirstOpp = getncaaopponent($leaguedb, $FirstTeamColl,$gameweek);
    if ($SecondTeamCollRow['lgteamid'])
        $SecondOpp = getncaaopponent($leaguedb, $SecondTeamCollRow['lgteamid'],$gameweek);
    $scorereport_content .= "<TR><TD width=\"38%\">$FirstPlayer</TD><TD width=\"7%\" align=\"center\"><div class=\"cluetip-title\" title=\"$FirstPlayer|The body of the title\">$FirstPlayerPoints</div></TD>";
    $scorereport_content .= "<TD width=\"10%\" align=\"center\">$position</TD><TD width=\"38%\">$SecondPlayer</TD><TD width=\"7%\" align=\"center\">$SecondPlayerPoints</TD></tr>";
    /* to here */			$FirstTeamScore = $FirstTeamScore + $FirstPlayerPoints;
                $SecondTeamScore = $SecondTeamScore + $SecondPlayerPoints;
            }
            if ($league_homefieldadv)
            {
                $scorereport_content .=  "<TR><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\"></TD><TD width=\"10%\" align=\"center\">HFA</TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$league_homefieldadv</TD></tr>";
                $SecondTeamScore+= $league_homefieldadv;
            }
            if ($FirstTeamScore > $SecondTeamScore)
            {
                $scorereport_content .=  "<TR bgcolor=\"#999999\"><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\"><b>$FirstTeamScore</b></TD><TD width=\"10%\" align=\"center\"><B>Total</B></TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$SecondTeamScore</TD></TR>";
            }
            elseif ($FirstTeamScore < $SecondTeamScore)
            {
                $scorereport_content .=  "<TR bgcolor=\"#999999\"><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$FirstTeamScore</TD><TD width=\"10%\" align=\"center\"><B>Total</B></TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\"><b>$SecondTeamScore</b></TD></TR>";
            }
            elseif ($leagueid == 3 || $leagueid ==5)
            {
                $firstteamtiebreakerquery = "SELECT players.playerid as playerid, lastname, firstname, lgteamabbr, players.pos, posorder, points, lgteams.lgteamid as lgteamid FROM players, positionorder, playerlineups, lgteams WHERE week={$gameweek['week']} AND playerlineups.playerid = players.playerid AND playerlineups.starter=2 AND playerlineups.teamid=$FirstTeam AND positionorder.pos = players.pos AND players.lgteamid = lgteams.lgteamid AND year !='Pro' AND positionorder.leagueid = $leagueid";
                $secondteamtiebreakerquery = "SELECT players.playerid as playerid, lastname, firstname, lgteamabbr, players.pos, posorder, points, lgteams.lgteamid as lgteamid FROM players, positionorder, playerlineups, lgteams WHERE week={$gameweek['week']} AND playerlineups.playerid = players.playerid AND playerlineups.starter=2 AND playerlineups.teamid=$SecondTeam AND positionorder.pos = players.pos AND players.lgteamid = lgteams.lgteamid AND year !='Pro' AND positionorder.leagueid = $leagueid";
                $firstteamtiebreaker = $leaguedb->queryRow($firstteamtiebreakerquery);
                DBerror($firstteamtiebreakerquery, $firstteamtiebreaker);
                $secondteamtiebreaker = $leaguedb->queryRow($secondteamtiebreakerquery);
                DBerror($secondteamtiebreakerquery, $secondteamtiebreaker);
                $firsttieplayer = "{$firstteamtiebreaker['pos']} {$firstteamtiebreaker['firstname']} {$firstteamtiebreaker['lastname']} - {$firstteamtiebreaker['lgteamabbr']}";
                $secondtieplayer = "{$secondteamtiebreaker['pos']} {$secondteamtiebreaker['firstname']} {$secondteamtiebreaker['lastname']} - {$secondteamtiebreaker['lgteamabbr']}";
                $scorereport_content .= "<TR><TD width=\"38%\">$firsttieplayer</TD><TD width=\"7%\" align=\"center\"><div class=\"cluetip-title\" title=\"$firsttieplayer|The body of the title\">{$firstteamtiebreaker['points']}</div></TD>";
                $scorereport_content .= "<TD width=\"10%\" align=\"center\">TIE</TD><TD width=\"38%\">$secondtieplayer</TD><TD width=\"7%\" align=\"center\">{$secondteamtiebreaker['points']}</TD></tr>";
                $FirstTeamScore += $firstteamtiebreaker['points'];
                $SecondTeamScore += $secondteamtiebreaker['points'];
                if ($FirstTeamScore > $SecondTeamScore)
                {
                    $scorereport_content .=  "<TR bgcolor=\"#999999\"><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\"><b>$FirstTeamScore</b></TD><TD width=\"10%\" align=\"center\"><B>Total</B></TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$SecondTeamScore</TD></TR>";
                }
                elseif ($FirstTeamScore < $SecondTeamScore)
                {
                    $scorereport_content .=  "<TR bgcolor=\"#999999\"><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$FirstTeamScore</TD><TD width=\"10%\" align=\"center\"><B>Total</B></TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\"><b>$SecondTeamScore</b></TD></TR>";
                }
                else
                {
                    $scorereport_content .=  "<TR bgcolor=\"#999999\"><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$FirstTeamScore</TD><TD width=\"10%\" align=\"center\"><B>Total</B></TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$SecondTeamScore</TD></TR>";
                }
            }
			else
			{
                $scorereport_content .=  "<TR bgcolor=\"#999999\"><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$FirstTeamScore</TD><TD width=\"10%\" align=\"center\"><B>Total</B></TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$SecondTeamScore</TD></TR>";
			}
        }
		if ($league_includepro)
		{
			$scorereport_content .=  "<TR><TD></TD><TD></TD><TD align=\"center\"><em>Pro</em></TD><TD></TD><TD></TD></TR>";
			$FirstTeamProStatsQuery = "SELECT nflplayers.playerid as playerid, lastname, firstname, lgteamabbr, nflplayers.pos, posorder, points, lgteams.lgteamid as lgteamid FROM nflplayers, positionorder, playerlineups, lgteams WHERE week={$gameweek['week']} AND playerlineups.playerid = nflplayers.playerid AND playerlineups.starter=1 AND playerlineups.teamid=$FirstTeam AND positionorder.pos = nflplayers.pos AND nflplayers.lgteamid = lgteams.lgteamid AND positionorder.leagueid = $leagueid order by posorder, lastname";
			$FirstTeamProStatsResult = $leaguedb->query($FirstTeamProStatsQuery );
			DBerror($FirstTeamProStatsQuery,$FirstTeamProStatsResult);
			$FirstTeamProStatsNumrows = $FirstTeamProStatsResult->numrows();
			$SecondTeamProStatsQuery = "SELECT nflplayers.playerid as playerid, lastname, firstname, lgteamabbr, nflplayers.pos, posorder, points, lgteams.lgteamid as lgteamid FROM nflplayers, positionorder, playerlineups, lgteams WHERE week={$gameweek['week']} AND playerlineups.playerid = nflplayers.playerid AND playerlineups.starter=1 AND playerlineups.teamid=$SecondTeam AND positionorder.pos = nflplayers.pos AND nflplayers.lgteamid = lgteams.lgteamid AND positionorder.leagueid = $leagueid order by posorder, lastname";
//            if ($SecondTeam == 144)
//                echo $SecondTeamProStatsQuery;
			$SecondTeamProStatsResult = $leaguedb->query($SecondTeamProStatsQuery );
			DBerror($SecondTeamProStatsQuery,$SecondTeamProStatsResult);
			$SecondTeamProStatsNumrows = $SecondTeamProStatsResult->numrows();
			if ($FirstTeamProStatsNumrows == 0 && $SecondTeamProStatsNumrows == 0)
			{
				$scorereport_content .=  "<TR><TD colspan=\"5\" align=\"center\">No score reported yet</TD></TR>";
			}
			else
			{
				$FirstTeamScore = 0;
				$SecondTeamScore = 0;
				$maxrows = max($FirstTeamProStatsNumrows,$SecondTeamProStatsNumrows);
				for ($i=0; $i<$maxrows; $i++)
				{
					$first_completions = 0;
					$first_attempts = 0;
					$first_passyds =0;
					$first_passtds=0;
					$first_passints=0;
					$first_carries=0;
					$first_rushyds=0;
					$first_rushtds=0;
					$first_recepts=0;
					$first_recyds=0;
					$first_rectds=0;
					$first_xp2=0;
					$first_kickreturntds=0;
					$first_puntreturntds=0;
					$first_pointsallow=0;
					$first_defwin = 0;
					$first_ydsallow = 0;
					$first_deftd=0;
					$first_specialteamtd=0;
					$first_safety=0;
					$first_sacks=0;
					$first_intercepts=0;
					$first_fumblerec=0;
					$first_fgmade=0;
					$first_patmade=0;
					$second_completions = 0;
					$second_attempts = 0;
					$second_passyds =0;
					$second_passtds=0;
					$second_passints=0;
					$second_carries=0;
					$second_rushyds=0;
					$second_rushtds=0;
					$second_recepts=0;
					$second_recyds=0;
					$second_rectds=0;
					$second_xp2=0;
					$second_kickreturntds=0;
					$second_puntreturntds=0;
					$second_pointsallow=0;
					$second_defwin = 0;
					$second_ydsallow = 0;
					$second_deftd=0;
					$second_specialteamtd=0;
					$second_safety=0;
					$second_sacks=0;
					$second_intercepts=0;
					$second_fumblerec=0;
					$second_fgmade=0;
					$second_patmade=0;
                    if ($FirstTeamProStatsNumrows != 0)
                    {
                        $FirstTeamProRow = $FirstTeamProStatsResult->fetchrow(MDB2_FETCHMODE_ASSOC, $i);
                        $FirstPlayerID = $FirstTeamProRow['playerid'];
                        $FirstTeamPro = $FirstTeamProRow['lgteamid'];
                        if ($FirstTeamProRow['pos'] != 'D')
                        {
                            $FirstPlayer = $FirstTeamProRow['firstname'].' '.$FirstTeamProRow['lastname'].' - '.$FirstTeamProRow['lgteamabbr'];
                        }
                        else
                        {
                            $FirstPlayer = $FirstTeamProRow['firstname'].' '.$FirstTeamProRow['lastname'];
                        }
                        $FirstPlayerPoints = $FirstTeamProRow['points'];
                        $firststatdetailquery = "SELECT * FROM playerstats WHERE playerid = $FirstPlayerID AND week = $gameweek{['week']}";
                        $firststatdetail = $leaguedb->queryRow($firststatdetailquery);
                        if (is_array($firststatdetail))
                            extract ($firststatdetail, EXTR_PREFIX_ALL, "first");
                        $position = $FirstTeamProRow['pos'];
                        if ($FirstTeamPro)
                            $FirstOpp = getncaaopponent($leaguedb, $FirstTeam,$gameweek['week']);
                    }
                    else
                    {
                        $FirstPlayer = "None selected";
                        $FirstPlayerPoints = 0;
                    }
                    if ($SecondTeamProStatsNumrows != 0)
                    {
						$SecondTeamProRow = $SecondTeamProStatsResult->fetchrow(MDB2_FETCHMODE_ASSOC, $i);
				        DBerror($SecondTeamProRow,$SecondTeamProStatsResult);
						$SecondPlayerID = $SecondTeamProRow['playerid'];
						if ($SecondTeamProRow['pos'] != 'D')
						{
							$SecondPlayer = $SecondTeamProRow['firstname'].' '.$SecondTeamProRow['lastname'].' - 	'.$SecondTeamProRow['lgteamabbr'];
						}
						else
						{
							$SecondPlayer = $SecondTeamProRow['firstname'].' '.$SecondTeamProRow['lastname'];
						}
						$SecondPlayerPoints = $SecondTeamProRow['points'];
						$secondstatdetailquery = "SELECT * FROM playerstats WHERE playerid = $SecondPlayerID AND week = $gameweek{['week']}";
						$secondstatdetail = $leaguedb->queryRow($secondstatdetailquery);
						if (is_array($secondstatdetail))
							extract ($secondstatdetail, EXTR_PREFIX_ALL, "second");
					}
                    else
                    {
                        $SecondPlayer = "None selected";
                        $SecondPlayerPoints = 0;
                    }
					
		if ($SecondTeamProRow['lgteamid'])
			$SecondOpp = getncaaopponent($leaguedb, $SecondTeamProRow['lgteamid'],$gameweek['week']);
		$scorereport_content .= "<TR><TD width=\"38%\">$FirstPlayer</TD><TD width=\"7%\" align=\"center\">$FirstPlayerPoints</TD>";
		$scorereport_content .= "<TD width=\"10%\" align=\"center\">$position</TD>";
		$scorereport_content .= "<TD width=\"38%\">$SecondPlayer</TD><TD width=\"7%\" align=\"center\">$SecondPlayerPoints</TD></tr>";
					
		/* to here */	$FirstTeamScore = $FirstTeamScore + $FirstPlayerPoints;
					$SecondTeamScore = $SecondTeamScore + $SecondPlayerPoints;
				}
				if ($league_homefieldadv)
				{
					$scorereport_content .=  "<TR><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\"></TD><TD width=\"10%\" align=\"center\">HFA</TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$league_homefieldadv</TD></tr>";
					$SecondTeamScore+= $league_homefieldadv;
				}
				if ($FirstTeamScore > $SecondTeamScore)
				{
				$scorereport_content .=  "<TR bgcolor=\"#999999\"><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\"><b>$FirstTeamScore</b></TD><TD width=\"10%\" align=\"center\"><B>Total</B></TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$SecondTeamScore</TD></TR>";
				}
				else
				{
				$scorereport_content .=  "<TR bgcolor=\"#999999\"><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\">$FirstTeamScore</TD><TD width=\"10%\" align=\"center\"><B>Total</B></TD><TD width=\"38%\"></TD><TD width=\"7%\" align=\"center\"><b>$SecondTeamScore</b></TD></TR>";
				}
			}
        }
    }
    $scorereport_content .=  "</TABLE>";
    $scorereport_content .= "<script>initThis();</script>";
    return $scorereport_content;
}
?>
