<?php
// +--------------------------------------------------------------------------+
// | GUS Plugin for glFusion CMS                                              |
// +--------------------------------------------------------------------------+
// | index.php                                                                |
// | Main index page for GUS public interface                                 |
// +--------------------------------------------------------------------------+
// | $Id::                                                                   $|
// +--------------------------------------------------------------------------+
// | Copyright (C) 2008-2011 by the following authors:                        |
// |                                                                          |
// | Mark R. Evans          mark AT glfusion DOT org                          |
// |                                                                          |
// | Based on the GUS Plugin for Geeklog                                      |
// | Copyright (C) 2002, 2003, 2005 by the following authors:                 |
// |                                                                          |
// | Authors: Andy Maloney      - asmaloney@users.sf.net                      |
// |          Tom Willett       - twillett@users.sourceforge.net              |
// |          John Hughes       - jlhughes@users.sf.net                       |
// |          Danny Ledger      - squatty@users.sourceforge.net               |
// +--------------------------------------------------------------------------+
// |                                                                          |
// | This program is free software; you can redistribute it and/or            |
// | modify it under the terms of the GNU General Public License              |
// | as published by the Free Software Foundation; either version 2           |
// | of the License, or (at your option) any later version.                   |
// |                                                                          |
// | This program is distributed in the hope that it will be useful,          |
// | but WITHOUT ANY WARRANTY; without even the implied warranty of           |
// | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            |
// | GNU General Public License for more details.                             |
// |                                                                          |
// | You should have received a copy of the GNU General Public License        |
// | along with this program; if not, write to the Free Software Foundation,  |
// | Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.          |
// |                                                                          |
// +--------------------------------------------------------------------------+

require_once './include/security.inc';

if ( !GUS_HasAccess( true ) )
	exit;

require_once './include/sql.inc';
require_once './include/util.inc';

/*
* Main Function
*/
$rec = DB_query( "SELECT DISTINCT YEAR( date ) AS year,
		MONTH( date ) AS month,
		DATE_FORMAT( date, '%b %Y' ) AS display_month
		FROM {$_TABLES['gus_userstats']} ORDER BY date" );

$rnum       = DB_numRows($rec);
$GUS_MONTHS = array();
$num_pages  = ceil( $rnum / $_GUS_months );

if ( !isset( $_GET['page'] ) || empty( $_GET['page'] ) )
    $curpage = 1;
else if ( $_GET['page'] == 'latest' )
	$curpage = $num_pages;
else
    $curpage = COM_applyFilter($_GET['page'],true);

settype( $curpage, 'integer' );

$base_url = GUS_create_url( 'page' );
$navlinks = COM_printPageNavigation( $base_url, $curpage, $num_pages );

for ( $i = 0; $i < $rnum; $i++ )
{
    $A = DB_fetchArray($rec);
    if (($i >= (($curpage - 1) * $_GUS_months)) && ($i < ($curpage * $_GUS_months)))
	{
		$GUS_MONTHS[] = $A;
    }
}

// First check for cached version

if (($curpage != $num_pages) && (file_exists(GUS_cachefile()))) {
    $display = GUS_getcache();
} else {
    // no cached version found do it.

    if ( SEC_inGroup( 'Root' ) || SEC_hasRights( 'gus.view' ) ) {
    	$T = GUS_template_start( 'index.thtml' );
    } else {
    	$T = GUS_template_start( 'index-a.thtml' );
    }

    $T->set_block('page','MONTH','ABlock');

    $T->set_var('stats_name', 'gus' );
    $T->set_var('site_url',$_CONF['site_url']);

    $T->set_var('month_title',$LANG_GUS00['month_title']);
    $T->set_var('anon_title',$LANG_GUS00['anon_title']);
    $T->set_var('reg_title',$LANG_GUS00['reg_title']);
    $T->set_var('page_title',$LANG_GUS00['page_title']);
    $T->set_var('story_title',$LANG_GUS00['new_stories']);
    $T->set_var('comm_title',$LANG_GUS00['new_comments']);
    $T->set_var('link_title',$LANG_GUS00['link_title']);
    $T->set_var('hour_title',$LANG_GUS00['hour_title']);
    $T->set_var('referer_title',$LANG_GUS00['referer_title']);
    $T->set_var('country_title',$LANG_GUS00['country_title']);
    $T->set_var('browser_title',$LANG_GUS00['browser_title']);
    $T->set_var('platform_title',$LANG_GUS00['platform_title']);

    $anon=0;
    $reg=0;
    $pages=0;
    $stories=0;
    $comments=0;
    $linksf=0;
    $referers=0;
    $countries=0;
    $rowNum = 1;

    foreach ( $GUS_MONTHS as $res )
    {
        $T->set_var( 'display_month', $res['display_month'] );
        $T->set_var( 'year', $res['year'] );
        $T->set_var( 'month', $res['month'] );

        if ( $rowNum % 2 )
    	    $T->set_var( 'rowclass', 'row1' );
    	else
    	    $T->set_var( 'rowclass', 'row2' );

    	$rowNum++;

    	$temp_table = GUS_create_temp_userstats_table( $res['year'], $res['month'] );

        $result = DB_query( "SELECT COUNT( DISTINCT ip ) AS num_anon FROM {$temp_table['name']} WHERE uid='1'" );
    	$row = DB_fetchArray( $result, false );
        $anon += $row['num_anon'];
        $T->set_var('anon', $row['num_anon']);

        $result = DB_query( "SELECT COUNT( DISTINCT uid ) AS num_registered FROM {$temp_table['name']} WHERE uid>'1'" );
    	$row = DB_fetchArray( $result, false );
        $reg += $row['num_registered'];
        $T->set_var('reg', $row['num_registered']);

    	$result = DB_query( "SELECT COUNT(*) AS num_pages FROM {$temp_table['name']}" );
    	$row = DB_fetchArray( $result, false );
        $pages += $row['num_pages'];
        $T->set_var('pages', $row['num_pages']);

    	$date_compare = GUS_get_date_comparison( 'date', $res['year'], $res['month'] );

    	$result = DB_query( "SELECT COUNT(*) AS num_stories FROM {$_TABLES['stories']} WHERE {$date_compare}" );
    	$row = DB_fetchArray( $result, false );
        $stories += $row['num_stories'];
        $T->set_var('stories', $row['num_stories']);

    	$result = DB_query( "SELECT COUNT(*) AS num_comments FROM {$_TABLES['comments']} WHERE {$date_compare}" );
    	$row = DB_fetchArray( $result, false );
        $comments += $row['num_comments'];
        $T->set_var('comments', $row['num_comments']);

        if ( $_GUS_phplinks == 1 )
        {
    		$outer_frame = DB_getItem($_TABLES['plsettings'], 'OuterFrame',"ID = '1' LIMIT 1" );

    		if ($outer_frame == "N")
    		{
    			$result = DB_query( "SELECT COUNT(*) AS num_links FROM {$temp_table['name']} WHERE page='phplinks/out.php'" );
    			$row = DB_fetchArray( $result, false );
    			$linksf += $row['num_links'];
    		}
    		else
    		{
    			$result = DB_query( "SELECT COUNT(*) AS num_links FROM {$temp_table['name']} WHERE page='phplinks/out_frame.php'" );
    			$row = DB_fetchArray( $result, false );
    			$linksf += $row['num_links'];
    		}
        }
        else
        {
    		$result = DB_query( "SELECT COUNT(*) AS num_links FROM {$temp_table['name']}
    							WHERE page LIKE '%portal.php' AND query_string <> ''" );
    		$row = DB_fetchArray( $result, false );
    		$linksf += $row['num_links'];
        }
        $T->set_var('linksf', $row['num_links']);

        $T->set_var('byhour','X');

    	$result = DB_query( "SELECT COUNT(*) AS num_referrers FROM {$temp_table['name']}
    						WHERE referer <> ''" );
    	$row = DB_fetchArray( $result, false );
        $referers += $row['num_referrers'];
        $T->set_var('referer', $row['num_referrers']);

        $rec1 = DB_query( "SELECT RIGHT( host, INSTR( REVERSE( host ), '.' ) - 1 ) AS country
        		FROM {$temp_table['name']} WHERE host <> 'localhost' AND ASCII( REVERSE( host ) ) > 64 GROUP BY country" );
        $num_rows = DB_numRows( $rec1 );
        $countries += $num_rows;
        $T->set_var('country', $num_rows);

        $T->set_var('browser','X');
        $T->set_var('platform','X');
        $T->Parse('ABlock','MONTH',true);

    	GUS_remove_temp_table( $temp_table );
    }

    $T->set_var('display_month',$LANG_GUS00['total']);
    $T->set_var('anon', $anon);
    $T->set_var('reg', $reg);
    $T->set_var('pages',$pages);
    $T->set_var('stories',$stories);
    $T->set_var('comments',$comments);
    $T->set_var('linksf',$linksf);
    $T->set_var('byhour','NA');
    $T->set_var('referer',$referers);
    $T->set_var('country',$countries);
    $T->set_var('browser','NA');
    $T->set_var('platform','NA');
    $T->set_var('google_paging',$navlinks);

    $display = GUS_template_finish( $T, $LANG_GUS00['monthly_title'] );

    if (($_GUS_cache) && ($curpage != $num_pages)) {
        GUS_writecache($display);
    }
}

echo COM_siteHeader( $_GUS_CONF['show_left_blocks'] );
echo $display;
echo COM_siteFooter( $_GUS_CONF['show_right_blocks'] );
?>