DROP TABLE IF EXISTS `monitord_fms`;
CREATE TABLE  `monitord_fms` (
  `id` int(11) NOT NULL auto_increment,
  `uhrzeit` datetime NOT NULL,
  `status` smallint(2) unsigned default NULL,
  `kennung` varchar(9) collate latin1_german1_ci NOT NULL,
  `richtung` char(10) collate latin1_german1_ci NOT NULL,
  `text` varchar(255) collate latin1_german1_ci NOT NULL,
  `tki` char(1) collate latin1_german1_ci NOT NULL default '',
  `quelle` varchar(2) collate latin1_german1_ci NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=0 DEFAULT CHARSET=latin1 COLLATE=latin1_german1_ci;

DROP TABLE IF EXISTS `monitord_pocsag`;
CREATE TABLE  `monitord_pocsag` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `uhrzeit` datetime NOT NULL,
  `kennung` varchar(45) collate latin1_german1_ci NOT NULL,
  `sub` varchar(45) collate latin1_german1_ci NOT NULL,
  `text` varchar(500) collate latin1_german1_ci NOT NULL,
  `quelle` tinyint(2) unsigned NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=0 DEFAULT CHARSET=latin1 COLLATE=latin1_german1_ci;

DROP TABLE IF EXISTS `monitord_zvei`;
CREATE TABLE  `monitord_zvei` (
  `id` int(11) NOT NULL auto_increment,
  `uhrzeit` datetime NOT NULL,
  `kennung` varchar(5) collate latin1_german1_ci NOT NULL,
  `typ` char(1) collate latin1_german1_ci NOT NULL,
  `text` varchar(80) collate latin1_german1_ci NOT NULL,
  `quelle` varchar(2) collate latin1_german1_ci NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=0 DEFAULT CHARSET=latin1 COLLATE=latin1_german1_ci;