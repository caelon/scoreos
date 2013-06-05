<?php
function rostersncaa_all()
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
	$leagueidquery = "SELECT leagueid FROM leagues WHERE leagueurl = '$leagueurl'";
	$leagueid = $leaguedb->queryOne($leagueidquery);
	DBerror($leagueidquery, $leagueid);

	$teamquery = "SELECT lgteamid, lgteamname, lgprimcolor, lgseccolor FROM lgteams WHERE lgteamid <= 124 ORDER BY lgteamname";
	$teamresult = $leaguedb->query($teamquery);
	DBerror($teamquery, $teamresult);
	
	$teampath = "http://".$leagueurl."/scoreos/teaminfo";
	$rostercontent = "<TABLE>";
	while ($teamrow = $teamresult->fetchRow())
	{
		extract($teamrow,EXTR_PREFIX_ALL,"left");

		$teamrow = $teamresult->fetchRow();
		extract($teamrow,EXTR_PREFIX_ALL,"right");
        
        $left_lgprimcolor = '000000';
        $left_lgseccolor = 'FFFFFF';
        $right_lgprimcolor = '000000';
        $right_lgseccolor = 'FFFFFF';

		$rostercontent .= "<TR><TH class=\"teamheader\" align=\"center\" bgcolor=\"#$left_lgprimcolor\"><font color=\"#$left_lgseccolor\">$left_lgteamname</TH><TH class=\"teamheader\" align=\"center\" bgcolor=\"#$right_lgprimcolor\"><font color=\"#$right_lgseccolor\">$right_lgteamname</TH></TR>";
		$LeftTeamQuery = "SELECT lastname, firstname, players.pos as pos, teamfirstname, year FROM players, positionorder, lgteams, playerteams, teams WHERE lgteams.lgteamid=$left_lgteamid AND positionorder.pos = players.pos AND players.playerid = playerteams.playerid AND players.lgteamid = lgteams.lgteamid AND playerteams.teamid = teams.teamid AND teams.leagueid = $leagueid AND positionorder.leagueid = $leagueid ORDER BY lastname, firstname
";
		$LeftTeamResult = $leaguedb->queryAll($LeftTeamQuery );
		//print_r($LeftTeamResult);
		
		$RightTeamQuery = "SELECT lastname, firstname, players.pos as pos, teamfirstname, year FROM players, positionorder, lgteams, playerteams, teams WHERE lgteams.lgteamid=$right_lgteamid AND positionorder.pos = players.pos AND players.playerid = playerteams.playerid AND players.lgteamid = lgteams.lgteamid AND playerteams.teamid = teams.teamid AND teams.leagueid = $leagueid AND positionorder.leagueid = $leagueid  ORDER BY lastname, firstname
";
		$RightTeamResult = $leaguedb->queryAll($RightTeamQuery );

		$i = 0;
		while ($LeftTeamResult[$i] || $RightTeamResult[$i])
		{
			if ($LeftTeamResult[$i])
			{
				$leftplayer = $LeftTeamResult[$i]['lastname'].", ".$LeftTeamResult[$i]['firstname']." - ".$LeftTeamResult[$i]['pos']." - ".$LeftTeamResult[$i]['teamfirstname']." (".$LeftTeamResult[$i]['year'].")";
				$rostercontent .= "<TR><TD>$leftplayer</TD>";
			}
			else
			{
				$rostercontent .= "<TR><TD></TD>";
			}
			if ($RightTeamResult[$i])
			{
				$rightplayer = $RightTeamResult[$i]['lastname'].", ".$RightTeamResult[$i]['firstname']." - ".$RightTeamResult[$i]['pos']." - ".$RightTeamResult[$i]['teamfirstname']." (".$RightTeamResult[$i]['year'].")";
				$rostercontent .= "<TD>$rightplayer</TD></tr>";
			}
			else
			{
				$rostercontent .= "<TD></TD></tr>";
			}
			$i++;
		}
	}
	$rostercontent .= "</TABLE>";
	return $rostercontent;
}
?>
