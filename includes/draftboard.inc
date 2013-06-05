<?php
function draftboard_all()
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

	$teamquery = "SELECT c.pick as pick, b.teamfirstname as teamname FROM teams b, draftpicks c where b.teamid = c.teamid order by pick";
	$teamresult = $leaguedb->query($teamquery);
	DBerror($teamquery, $teamresult);
	
	while ($teamrow = $teamresult->fetchrow())
	{
		extract($teamrow);
		$draft[$pick]['teamname'] = $teamname;
	}
	
	$teamquery = "SELECT c.pick as pick, a.firstname as firstname, a.lastname as lastname, a.pos as playerpos, b.teamfirstname as teamname, d.lgteamname as lgteamname FROM players a, teams b, draftpicks c, lgteams d where a.playerid = c.playerid and b.teamid = c.teamid and a.lgteamid = d.lgteamid order by pick";
	$teamresult = $leaguedb->query($teamquery);
	DBerror($teamquery, $teamresult);
	
	while ($teamrow = $teamresult->fetchrow())
	{
		extract($teamrow);
		$draft[$pick]['teamname'] = $teamname;
		$draft[$pick]['name'] = $firstname." ".$lastname." - ".$playerpos." - ".$lgteamname;
	}
	
	$teamquery = "SELECT c.pick as pick, a.firstname as firstname, a.lastname as lastname, a.pos as playerpos, b.teamfirstname as teamname, d.lgteamname as lgteamname FROM nflplayers a, teams b, draftpicks c, lgteams d where a.playerid = c.playerid and b.teamid = c.teamid and a.lgteamid = d.lgteamid order by pick";
	$teamresult = $leaguedb->query($teamquery);
	DBerror($teamquery, $teamresult);
	
	while ($teamrow = $teamresult->fetchrow())
	{
		extract($teamrow);
		$draft[$pick]['teamname'] = $teamname;
		$draft[$pick]['name'] = $firstname." ".$lastname." - ".$playerpos." - ".$lgteamname;
	}
	
	$rostercontent = "<TABLE>";
	for ($count=1; $count<=420; $count++)
	{
		$leftround = (floor(($count-1)/14))+1;
		$leftpick = $count%14;
		if ($leftpick == 0)
		{
			$leftpick = 14;
		}
		$rostercontent .= "<TR><Td class=\"teampick\" align=\"center\">$leftround.$leftpick</Td><Td class=\"player\" align=\"left\">".$draft[$count]['teamname']."</Td><Td class=\"player\" align=\"left\">".$draft[$count]['name']."</Td></TR>";
		if ($leftpick == 14)
		{
			$rostercontent .= "<TR><TD colspan=3 bgcolor=\"000000\">&nbsp;</td></tr>";
		}
	}
	$rostercontent .= "</TABLE>";
	return $rostercontent;
}
?>
