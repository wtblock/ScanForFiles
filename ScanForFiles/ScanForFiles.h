/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"
#include "KeyedCollection.h"
#include <vector>

using namespace std;

// the root folder
CString m_csRoot;
bool m_bDuplicatesOnly;

// collection of unique filenames and the folders they are in
CKeyedCollection<CString,vector<CString> > m_mapFilenames;
