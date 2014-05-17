<?php
// +--------------------------------------------------------------------------+
// | GUS Plugin for glFusion CMS                                              |
// +--------------------------------------------------------------------------+
// | daily.php                                                                |
// | Displays daily usage stats                                               |
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

if ( !GUS_HasAccess() )
	exit;

require_once './include/sql.inc';
require_once './include/util.inc';

if ( ($day == 0) && ($month == 0) && ($year == 0) ) {
	$year   = date( 'Y' );
	$month  = date( 'n' );
	$day    = date( 'j' );

	$sort_sep = '?';
}

/*
* Main Function
*/
// Check for cached file
if((file_exists(GUS_cachefile())) && (date("Yn") != $year . $month)) {
    $display = GUS_getcache();
} else {
    // no cached version found - generate page

    if ( SEC_inGroup( 'Root' ) || SEC_hasRights( 'gus.view' ) )
    	$T = GUS_template_start( 'daily.thtml' );
    else
    	$T = GUS_template_start( 'daily-a.thtml' );

    $T->set_var( 'additional_nav', GUS_make_nav( $day, $month, $year ) );

    $T->set_block('page','ROW','ABlock');
    $T->set_var('stats_name', 'gus' );

    $T->set_var( 'site_url', $_CONF['site_url'] );
    $T->set_var('period_title',$LANG_GUS00['day_title']);
    $T->set_var('anon_title',$LANG_GUS00['anon_title']);
    $T->set_var('reg_title',$LANG_GUS00['reg_title']);
    $T->set_var('page_title',$LANG_GUS00['page_title']);
    $T->set_var('story_title',$LANG_GUS00['new_stories']);
    $T->set_var('comm_title',$LANG_GUS00['new_comments']);
    $T->set_var('link_title',$LANG_GUS00['link_title']);
    $anon=0;
    $reg=0;
    $pages=0;
    $stories=0;
    $comments=0;
    $linksf=0;

    $days = Date( 't', mktime( 0, 0, 0, $month, 1, $year ) );

    // special case for this month - don't show days in the future
    $today = getdate();
    if ( ($today['year'] == $year) && ($today['mon'] == $month) )
    	$days = min( $days, $today['mday'] );

    $num_pages = ceil($days / $_GUS_days);

    if ( !isset( $_GET['page'] ) || empty( $_GET['page'] ) )
      $curpage = 1;
    else
      $curpage = $_GET['page'];

    settype( $curpage, 'integer' );

    $base_url = GUS_create_url('page');
    $navlinks = COM_printPageNavigation($base_url,$curpage,$num_pages);

    $temp_table = GUS_create_temp_userstats_table( $year, $month );

    for($day=1;$day<=$days;$day++)
    {
        if ( ($day > (($curpage - 1) * $_GUS_days)) && ($day <= ($curpage * $_GUS_days)) )
        {
            $day_of_week = Date( 'l', mktime( 0, 0, 0, $month, $day, $year ) );
            $date_formatted = Date( 'l d', mktime( 0, 0, 0, $month, $day, $year ) );
            $T->set_var( 'day_display', $date_formatted );

            $T->set_var( 'day', $day );
            $T->set_var( 'mon', $month );
            $T->set_var( 'year', $year );

    		if ( $day % 2 )
    			$T->set_var( 'rowclass', 'row1' );
    		else
    			$T->set_var( 'rowclass', 'row2' );

            $result = DB_query( "SELECT COUNT( DISTINCT ip ) AS num_anon FROM {$temp_table['name']} WHERE uid = '1' AND DAYOFMONTH( date ) = $day" );
            $row = DB_fetchArray( $result, false );
            $anon += $row['num_anon'];
            $T->set_var( 'anon', $row['num_anon'] );

            $result = DB_query( "SELECT COUNT( DISTINCT uid ) AS num_registered FROM {$temp_table['name']} WHERE uid > '1' AND DAYOFMONTH( date ) = $day" );
            $row = DB_fetchArray( $result, false );
            $reg += $row['num_registered'];
            $T->set_var( 'reg', $row['num_registered'] );

            $result = DB_query( "SELECT COUNT(*) AS num_pages FROM {$temp_table['name']} WHERE DAYOFMONTH( date ) = $day" );
            $row = DB_fetchArray( $result, false );
            $pages += $row['num_pages'];
            $T->set_var( 'pages', $row['num_pages'] );

            $date_compare = GUS_get_date_comparison( 'date', $year, $month, $day );

            $result = DB_query( "SELECT COUNT(*) AS num_stories FROM {$_TABLES['stories']} WHERE {$date_compare}" );
            $row = DB_fetchArray( $result, false );
            $stories += $row['num_stories'];
            $T->set_var( 'stories', $row['num_stories'] );

            $result = DB_query( "SELECT COUNT(*) AS num_comments FROM {$_TABLES['comments']} WHERE {$date_compare}" );
            $row = DB_fetchArray( $result, false );
            $comments += $row['num_comments'];
            $T->set_var( 'comments', $row['num_comments'] );

            $result = DB_query( "SELECT COUNT(*) AS num_links FROM {$temp_table['name']}
            					WHERE page LIKE '%portal.php' AND query_string <> '' AND DAYOFMONTH( date ) = $day" );
            $row = DB_fetchArray( $result, false );
            $linksf += $row['num_links'];
            $T->set_var( 'linksf', $row['num_links'] );

            $T->Parse( 'ABlock', 'ROW', true );
        }
    }

    $T->set_var('period',$LANG_GUS00['total']);
    $T->set_var('anon', $anon);
    $T->set_var('reg', $reg);
    $T->set_var('pages',$pages);
    $T->set_var('stories',$stories);
    $T->set_var('comments',$comments);
    $T->set_var('linksf',$linksf);
    $T->set_var('google_paging',$navlinks);

    $title = Date( 'F Y - ', mktime( 0, 0, 0, $month, 1, $year ) ) . $LANG_GUS00['daily_title'];

    $display = GUS_template_finish( $T, $title );

    if (($_GUS_cache) && ((date("Yn") != $year . $month)))
        GUS_writecache($display);

    GUS_remove_temp_table( $temp_table );
}

echo COM_siteHeader( $_GUS_CONF['show_left_blocks'] );
echo $display;
echo COM_siteFooter( $_GUS_CONF['show_right_blocks'] );
?>