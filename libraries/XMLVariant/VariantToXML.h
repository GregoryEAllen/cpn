
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
#pragma once
#include "Variant.h"
#include <string>
#include <iosfwd>

std::string VariantToXML(Variant v, const std::string &rname, bool pretty = false);

std::ostream &VariantToXML(std::ostream &os, const Variant &v, const std::string &rname, bool pretty = false);


