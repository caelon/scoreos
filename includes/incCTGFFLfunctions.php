<?PHP

function DBerror($DBquery, $DBresult)
// this function takes the query and result from a query and checks it for an error
{
    require_once("MDB2.php");
    if(PEAR::isError($DBresult)) 
    {
        die('Failed to issue query, error message : ' . $DBresult->getMessage()." on ".$DBquery);
    }
}

function array_csort() {  //coded by Ichier2003
    $args = func_get_args();
    $marray = array_shift($args);

    $msortline = "return(array_multisort(";
    foreach ($args as $arg) {
        $i++;
        if (is_string($arg)) {
            foreach ($marray as $row) {
                $sortarr[$i][] = $row[$arg];
            }
        } else {
            $sortarr[$i] = $arg;
        }
        $msortline .= "\$sortarr[".$i."],";
    }
    $msortline .= "\$marray));";

    eval($msortline);
    return $marray;
}

function stddev($std)
    {
    $total;
    while(list($key,$val) = each($std))
        {
        $total += $val;
        }
    reset($std);
    $mean = $total/count($std);

    while(list($key,$val) = each($std))
        {
        $sum += pow(($val-$mean),2);
        }
    $var = sqrt($sum/(count($std)-1));
    return $var;
    }

function ordinal($number) {

    // when fed a number, adds the English ordinal suffix. Works for any
    // number, even negatives

    if ($number % 100 > 10 && $number %100 < 14):
        $suffix = "th";
    else:
        switch($number % 10) {

            case 0:
                $suffix = "th";
                break;

            case 1:
                $suffix = "st";
                break;

            case 2:
                $suffix = "nd";
                break;

            case 3:
                $suffix = "rd";
                break;

            default:
                $suffix = "th";
                break;
        }

    endif;

    return "${number}<SUP>$suffix</SUP>";

}

function getPlayerName($playerid, $cffldb)
{
	$playernamequery = "SELECT firstname, lastname, lgteamname, pos, year FROM players, lgteams WHERE players.lgteamid = lgteams.lgteamid AND players.playerid = $playerid";
	$playernamerow =  $cffldb->queryRow($playernamequery);
	DBerror($playernamequery, $playernamerow);

    if(PEAR::isError($playernamerow) || !$playernamerow) 
    {
		$playernamequery = "SELECT lastname, firstname, lgteamname, pos, year FROM nflplayers, lgteams WHERE nflplayers.lgteamid = lgteams.lgteamid AND nflplayers.playerid = $playerid";
		$playernamerow =  $cffldb->queryRow($playernamequery);
		DBerror($playernamequery, $playernamerow);

		if(PEAR::isError($playernamerow) || !$playernamerow) 
		{
		    $playername = '';
		}
		else
		{
			$playername = $playernamerow['firstname']." ".$playernamerow['lastname']." - ".$playernamerow['pos']." - ".$playernamerow['lgteamname'];
		}
    }
    else
    {
        $playername = $playernamerow['firstname']." ".$playernamerow['lastname']." - ".$playernamerow['pos']." - ".$playernamerow['lgteamname'];
    }
	return $playername;
}

function getNFLPlayerName($playerid, $cffldb)
{
	$playernamequery = "SELECT lastname, firstname, lgteamname, pos, year FROM nflplayers, lgteams WHERE nflplayers.lgteamid = lgteams.lgteamid AND nflplayers.playerid = $playerid";
	$playernamerow =  $cffldb->queryRow($playernamequery);
	DBerror($playernamequery, $playernamerow);
    if(PEAR::isError($playernamenow)) 
    {
        $playername = '';
    }
    else
    {
        $playername = $playernamerow['firstname']." ".$playernamerow['lastname']." - ".$playernamerow['pos']." - ".$playernamerow['lgteamname'];
    }
	return $playername;
}

function getPlayerNameLastFirst($playerid, $cffldb)
{

	$playernamequery = "SELECT firstname, lastname, lgteamname, pos, year FROM players, lgteams WHERE players.lgteamid = lgteams.lgteamid AND players.playerid = $playerid";
	$playernamerow =  $cffldb->queryRow($playernamequery);
	DBerror($playernamequery, $playernamerow);

	if (!$playernamerow)
	{
		$playernamequery = "SELECT firstname, lastname, lgteamname, pos, year FROM nflplayers, lgteams WHERE nflplayers.lgteamid = lgteams.lgteamid AND nflplayers.playerid = $playerid";
		$playernamerow =  $cffldb->queryRow($playernamequery);
		DBerror($playernamequery, $playernamerow);
	}

	$playername = $playernamerow['lastname'].", ".$playernamerow['firstname']." - ".$playernamerow['pos']." - ".$playernamerow['lgteamname'];
	return $playername;
}

function emailLeague($subject,$message)
{
    //lots of work to do here
	$fromname = "CTGFFL System";
	$fromemail = "commish@ctgffl.com";
	$toname = "CTGFFL Owners";
//	$toemail = "caelon@gmail.com";
	$toemail = "sabreman75@hotmail.com, eldeacon@yahoo.com, caelon@gmail.com, bpdouglass@gmail.com, hcffl@yahoo.com, damien.arabie@gmail.com, leoilstop@yahoo.com, ghstryder@cfl.rr.com, tblaz@cox.net, fantasy@hyphencreative.com, will.pridemore@gmail.com, ucfknightsfan@gmail.com, fvosicky@roadrunner.com, govolskicksomeass@hotmail.com, osfanth@yahoo.com";

	$subject = "CTGFFL: ".$subject;

	$headers = 'From: '.$fromemail . "\r\n" .
            'Reply-To: '.$fromemail . "\r\n" .
                'X-Mailer: PHP/' . phpversion();
	date_default_timezone_set('America/New York');

	$mailmessage = "This is a system generated message from CTGFFL...\r\n".$message;

	mail($toemail, $subject, $mailmessage, $headers);
}

function emailPick($db, $picknumber, $chosenplayer, $pickmsg)
{
	$playername = getPlayerName($chosenplayer, $db);
	$currtime = getdate();
	$nextpickduemonth = $currtime['mon'];
	$nextpickdueday = $currtime['mday'];
	$nextpickduehour = $currtime['hours'];
	$nextpickduemin = $currtime['minutes'];
	if ($nextpickduehour >= 16)
	{
		$nextpickduehour -= 8;
		if ($nextpickdueday == 31)
		{
			$nextpickduemonth = 8;
			$nextpickdueday = 1;
		}
		else
		{
			$nextpickdueday += 1;
		}
	}
	else
	{
		$nextpickduehour += 8;
	}

	$nextpickdue = $nextpickduemonth."-".$nextpickdueday." at ".$nextpickduehour.":".$nextpickduemin;

	$numteams = 14;
	$roundofnextpick = ceil($picknumber/$numteams);
	$numberofnextpick = $picknumber%$numteams;
	if ($numberofnextpick == 0) $numberofnextpick = $numteams;
	$numberofnextpick = sprintf("%02s", $numberofnextpick);
	$roundpicknext = $roundofnextpick.".".$numberofnextpick;

	$teamnamequery = "SELECT teamfirstname FROM teams a, draftpicks b WHERE pick=$picknumber AND a.teamid = b.teamid";
	$teamname =  $db->queryOne($teamnamequery);
	DBerror($teamnamequery, $teamname);

	$picknumber++;

	$nextpickquery = "SELECT playerid FROM draftpicks WHERE pick=$picknumber";
	$nextpickplayer = $db->queryOne($nextpickquery);
	
	while ($nextpickplayer)
	{
		$picknumber++;
		$nextpickquery = "SELECT playerid FROM draftpicks WHERE pick=$picknumber";
		$nextpickplayer = $db->queryOne($nextpickquery);
	}

	$nextteamnamequery = "SELECT teamfirstname FROM teams a, draftpicks b WHERE pick=$picknumber AND a.teamid = b.teamid";
	$nextteamname =  $db->queryOne($nextteamnamequery);
	DBerror($nextteamnamequery, $nextteamname);

	$picknumber++;

	$nextpickquery = "SELECT playerid FROM draftpicks WHERE pick=$picknumber";
	$nextpickplayer = $db->queryOne($nextpickquery);
	
	while ($nextpickplayer)
	{
		$picknumber++;
		$nextpickquery = "SELECT playerid FROM draftpicks WHERE pick=$picknumber";
		$nextpickplayer = $db->queryOne($nextpickquery);
	}

	$deckteamnamequery = "SELECT teamfirstname FROM teams a, draftpicks b WHERE pick=$picknumber AND a.teamid = b.teamid";
	$deckteamname =  $db->queryOne($deckteamnamequery);
	DBerror($deckteamnamequery, $deckteamname);

	$picknumber++;

	$nextpickquery = "SELECT playerid FROM draftpicks WHERE pick=$picknumber";
	$nextpickplayer = $db->queryOne($nextpickquery);
	
	while ($nextpickplayer)
	{
		$picknumber++;
		$nextpickquery = "SELECT playerid FROM draftpicks WHERE pick=$picknumber";
		$nextpickplayer = $db->queryOne($nextpickquery);
	}

	$holeteamnamequery = "SELECT teamfirstname FROM teams a, draftpicks b WHERE pick=$picknumber AND a.teamid = b.teamid";
	$holeteamname =  $db->queryOne($holeteamnamequery);
	DBerror($holeteamnamequery, $holeteamname);

	$message = "With pick ".$roundpicknext.", the ".$teamname." select ".$playername.".\r\n\r\n";
	foreach ($pickmsg as $currmsg)
	{
		$message .= $currmsg."\r\n";
	}

	$message .= $nextteamname." are up and their pick is due ".$nextpickdue."\r\n";
	$message .= $deckteamname." are on deck.\r\n";
	$message .= $holeteamname." are in the hole.\r\n\r\n";
	$message .= "For the full draft report, go to www.ctgffl.com/scoreos/draftboard";

	$subject = "Draft Pick ".$roundpicknext." by ".$teamname;

	emailLeague($subject, $message);

}

function getncaaopponent($db, $team, $gameweek)
{
    $HomeScheduleQuery = "SELECT lgteamabbr, visitor FROM lgteams, ncaasched WHERE home=$team AND gamedate >= '{$gameweek['Thursday']}' AND gamedate <= '{$gameweek['Wednesday']}' AND visitor = lgteamid";
    $HomeScheduleResult = $db->query($HomeScheduleQuery );
    DBerror($HomeScheduleQuery, $HomeScheduleResult);
    $HomeScheduleNumrows = $HomeScheduleResult->numrows();
    if ($HomeScheduleNumrows)
    {
        $HomeScheduleRow = $HomeScheduleResult->fetchrow(DB_FETCHMODE_ASSOC);
        $gametext = " (vs. ".$HomeScheduleRow['lgteamabbr'].")";
    }
    else
    {
        $VisScheduleQuery = "SELECT lgteamabbr, home FROM lgteams, ncaasched WHERE visitor=$team AND gamedate >= '{$gameweek['Thursday']}' AND gamedate <= '{$gameweek['Wednesday']}' AND home=lgteamid";
        $VisScheduleResult = $db->query($VisScheduleQuery );
        DBerror($VisScheduleQuery, $VisScheduleResult);
        $VisScheduleNumrows = $VisScheduleResult->numrows();
        if ($VisScheduleNumrows)
        {
            $VisScheduleRow = $VisScheduleResult->fetchrow(DB_FETCHMODE_ASSOC);
            $gametext = " (at ".$VisScheduleRow['lgteamabbr'].")";
        }
        else
        {
            $gametext = " (BYE)";
        }
    }
}

function getTeamAbbr($db, $teamid)
{
    $abbrquery = "SELECT lgteamabbr FROM lgteams WHERE lgteamid = $teamid";
    $abbr = $db->queryOne($abbrquery);
    DBerror($abbrquery, $abbr);

    return $abbr;
}

function getLeagueWeek($week = NULL)
{
	$gameweek = array();
	if (!isset($week))
	{
		$week = date(W)-34;
	}
	$gameweek['week'] = $week;
	$gameweek['lastweek'] = $week-1;
	$gameweek['Mondayprev'] = date("Y-m-d",mktime(0,0,0,0,(($week+38)*7)-2,2012));
	$gameweek['Tuesdayprev'] = date("Y-m-d",mktime(0,0,0,0,(($week+38)*7)-1,2012));
	$gameweek['Wednesday'] = date("Y-m-d",mktime(0,0,0,0,(($week+38)*7),2012));
	$gameweek['Thursday'] = date("Y-m-d",mktime(0,0,0,0,(($week+38)*7)+1,2012));
	$gameweek['Saturday'] = date("Y-m-d",mktime(0,0,0,0,(($week+38)*7)+3,2012));
	$gameweek['Saturdayuri'] = date("Ymd",mktime(0,0,0,0,(($week+38)*7)+3,2012));
	$gameweek['Monday'] = date("Y-m-d",mktime(0,0,0,0,(($week+38)*7)+5,2012));
	$gameweek['Tuesday'] = date("Y-m-d",mktime(0,0,0,0,(($week+38)*7)+6,2012));
	$gameweek['today'] = date("Y-m-d");
	$gameweek['todaytime'] = date("G");
	
	return $gameweek;
}

function getScoringSystem($leaguedb, $leagueid)
{
    $scoringquery = "SELECT statname, amount FROM scoringsystem WHERE leagueid = $leagueid";
    $scoringresult = $leaguedb->query($scoringquery);
    DBerror($scoringquery, $scoringresult);	
	
	$scoring = array();
	while ($scoringrow = $scoringresult->fetchrow(MDB2_FETCHMODE_ASSOC))
	{
		$scoring[$scoringrow['statname']] = $scoringrow['amount'];
	}
	
	return $scoring;
}
?>
