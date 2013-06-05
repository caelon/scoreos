<?php
function rosters_all()
{
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
	DBerror($leagueidquery, $leagueid);
    $leagueid = $leagueidrow['leagueid'];
    $teaswr = $leagueidrow['teaswr'];
    $includepro = $leagueidrow['includepro'];

	$teampath = "http://".$leagueurl."/scoreos/teaminfo";

	$teamquery = "SELECT teamid, teamfirstname as teamname, primcolor, seccolor FROM teams WHERE active = 1 AND leagueid = $leagueid ORDER BY teamname";
	$teamresult = $leaguedb->query($teamquery);
	DBerror($teamquery, $teamresult);
	
	$rostercontent = "<TABLE>";
	while ($teamrow = $teamresult->fetchRow())
	{
		extract($teamrow,EXTR_PREFIX_ALL,"left");

		$teamrow = $teamresult->fetchRow();
		extract($teamrow,EXTR_PREFIX_ALL,"right");

		$rostercontent .= "<TR><TH class=\"teamheader\" align=\"center\" bgcolor=\"#$left_primcolor\"><a href=\"$teampath/$left_teamid\"><font color=\"#$left_seccolor\">$left_teamname</a></TH><TH class=\"teamheader\" align=\"center\" bgcolor=\"#$right_primcolor\"><a href=\"$teampath/$right_teamid\"<font color=\"#$right_seccolor\">$right_teamname</a></TH></TR>";
        if ($includepro)
        {
            $rostercontent .= "<TR><TD align=\"center\"><em>College</em></td><TD align=\"center\"><em>College</em></td></tr>";
        }
		$LeftTeamQuery = "SELECT lastname, firstname, players.pos as pos, lgteamname, year FROM players, positionorder, lgteams, playerteams WHERE  teamid=$left_teamid AND positionorder.pos = players.pos AND players.playerid = playerteams.playerid AND players.lgteamid = lgteams.lgteamid AND positionorder.leagueid = $leagueid ORDER BY posorder, lastname";
		$LeftTeamResult = $leaguedb->queryAll($LeftTeamQuery );
     	DBerror($LeftTeamQuery, $LeftTeamResult);

		$RightTeamQuery = "SELECT lastname, firstname, players.pos as pos, lgteamname, year FROM players, positionorder, lgteams, playerteams WHERE teamid=$right_teamid AND positionorder.pos = players.pos AND players.playerid = playerteams.playerid AND players.lgteamid = lgteams.lgteamid AND positionorder.leagueid = $leagueid ORDER BY posorder, lastname";
		$RightTeamResult = $leaguedb->queryAll($RightTeamQuery );
     	DBerror($RightTeamQuery, $RightTeamResult);

		$i = 0;
		while ($LeftTeamResult[$i] || $RightTeamResult[$i])
		{
			if ($LeftTeamResult[$i])
			{
				$leftplayer = $LeftTeamResult[$i]['lastname'].", ".$LeftTeamResult[$i]['firstname']." - ".$LeftTeamResult[$i]['pos']." - ".$LeftTeamResult[$i]['lgteamname']." (".$LeftTeamResult[$i]['year'].")";
				$rostercontent .= "<TR><TD>$leftplayer</TD>";
			}
			else
			{
				$rostercontent .= "<TR><TD></TD>";
			}
			if ($RightTeamResult[$i])
			{
				$rightplayer = $RightTeamResult[$i]['lastname'].", ".$RightTeamResult[$i]['firstname']." - ".$RightTeamResult[$i]['pos']." - ".$RightTeamResult[$i]['lgteamname']." (".$RightTeamResult[$i]['year'].")";
				$rostercontent .= "<TD>$rightplayer</TD></tr>";
			}
			else
			{
				$rostercontent .= "<TD></TD></tr>";
			}
			$i++;
		}
        if ($includepro)
        {
            $rostercontent .= "<TR><TD align=\"center\"><em>Pro</em></td><TD align=\"center\"><em>Pro</em></td></tr>";
            $LeftTeamQuery = "SELECT lastname, firstname, nflplayers.pos as pos, lgteamname, year FROM nflplayers, positionorder, lgteams, playerteams WHERE  teamid=$left_teamid AND positionorder.pos = nflplayers.pos AND nflplayers.playerid = playerteams.playerid AND nflplayers.lgteamid = lgteams.lgteamid AND positionorder.leagueid = $leagueid ORDER BY posorder, lastname";
            $LeftTeamResult = $leaguedb->queryAll($LeftTeamQuery );
            DBerror($LeftTeamQuery, $LeftTeamResult);

            $RightTeamQuery = "SELECT lastname, firstname, nflplayers.pos as pos, lgteamname, year FROM nflplayers, positionorder, lgteams, playerteams WHERE teamid=$right_teamid AND positionorder.pos = nflplayers.pos AND nflplayers.playerid = playerteams.playerid AND nflplayers.lgteamid = lgteams.lgteamid AND positionorder.leagueid = $leagueid ORDER BY posorder, lastname";
            $RightTeamResult = $leaguedb->queryAll($RightTeamQuery );
            DBerror($RightTeamQuery, $RightTeamResult);

            $i = 0;
            while ($LeftTeamResult[$i] || $RightTeamResult[$i])
            {
                if ($LeftTeamResult[$i])
                {
                    $leftplayer = $LeftTeamResult[$i]['lastname'].", ".$LeftTeamResult[$i]['firstname']." - ".$LeftTeamResult[$i]['pos']." - ".$LeftTeamResult[$i]['lgteamname'];
                    $rostercontent .= "<TR><TD>$leftplayer</TD>";
                }
                else
                {
                    $rostercontent .= "<TR><TD></TD>";
                }
                if ($RightTeamResult[$i])
                {
                    $rightplayer = $RightTeamResult[$i]['lastname'].", ".$RightTeamResult[$i]['firstname']." - ".$RightTeamResult[$i]['pos']." - ".$RightTeamResult[$i]['lgteamname'];
                    $rostercontent .= "<TD>$rightplayer</TD></tr>";
                }
                else
                {
                    $rostercontent .= "<TD></TD></tr>";
                }
                $i++;
            }
        }
	}
	$rostercontent .= "</TABLE>";
	return $rostercontent;
}
?>
