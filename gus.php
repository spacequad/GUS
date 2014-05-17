<?php

// +--------------------------------------------------------------------------+

// | GUS Plugin for glFusion CMS                                              |

// +--------------------------------------------------------------------------+

// | gus.php                                                                  |

// +--------------------------------------------------------------------------+

// | $Id::                                                                   $|

// +--------------------------------------------------------------------------+

// | Copyright (C) 2008-2011 by the following authors:                        |

// |                                                                          |

// | Mark R. Evans          mark AT glfusion DOT org                          |

// |                                                                          |

// | Based on the GUS Plugin for Geeklog CMS                                  |

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



if (!defined ('GVERSION')) {

    die ('This file can not be used on its own.');

}



$_GUS_CONF['pi_name']           = 'gus';

$_GUS_CONF['pi_display_name']   = 'GUS';

$_GUS_CONF['pi_version']        = '2.1.9';

$_GUS_CONF['gl_version']        = '1.4.1';

$_GUS_CONF['pi_url']            = 'http://www.glfusion.org';



$_GUS_table_prefix = $_DB_table_prefix . 'gus_';



$_TABLES['gus_userstats']      = $_GUS_table_prefix . 'userstats';

$_TABLES['gus_user_agents']    = $_GUS_table_prefix . 'user_agents';

$_TABLES['gus_ignore_ip']      = $_GUS_table_prefix . 'ignore_ip';

$_TABLES['gus_ignore_user']    = $_GUS_table_prefix . 'ignore_user';

$_TABLES['gus_ignore_page']    = $_GUS_table_prefix . 'ignore_page';

$_TABLES['gus_ignore_ua']      = $_GUS_table_prefix . 'ignore_ua';

$_TABLES['gus_ignore_host']    = $_GUS_table_prefix . 'ignore_host';

$_TABLES['gus_ignore_referrer']= $_GUS_table_prefix . 'ignore_referrer';

$_TABLES['gus_vars']           = $_GUS_table_prefix . 'vars';

?>
