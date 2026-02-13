//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 * Do a VERY simple translation from
 * Variant to XML that the XMLToVariant
 * parser will mostly translate back.
 *
 * Numbers, true, false
 * get turned into strings...
 *
 * This may also mangle the Variant represntation.
 * For example Arrays inside Arrays get manged.
 *
 * If the Variant was read in by XMLToVariant then
 * VariantToXML will produce something that XMLToVariant
 * will translate back nearly perfrectly.
 */
#ifndef VARIANTTOXML_H
#define VARIANTTOXML_H
#pragma once
#include "Variant.h"
#include <string>
#include <iosfwd>

/**
 * Transform a Variant to an XML representation and return it as a string.
 * \param v the Variant to transform
 * \param rname the name of the root node
 * \param pretty true pretty print, false not
 * \return the string
 */
std::string VariantToXML(Variant v, const std::string &rname, bool pretty = false);

/**
 * Transform a Variant to an XML representation and output it to an std::ostream
 * \param os the std::ostream to output to
 * \param v the Variant to transform
 * \param rname the roo tnode name
 * \param pretty pretty printing
 * \return the ostream
 */
std::ostream &VariantToXML(std::ostream &os, const Variant &v, const std::string &rname, bool pretty = false);

#endif
