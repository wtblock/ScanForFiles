/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScanForFiles.h"
#include <vector>
#include "CHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// crawl through the directory tree looking for supported image extensions
void RecursePath( LPCTSTR path )
{
	USES_CONVERSION;

	// get the folder which will trim any wild card data
	CString csPathname = CHelper::GetFolder( path );

	csPathname.TrimRight( _T( "\\" ) );
	CString strWildcard;

	strWildcard.Format( _T( "%s\\*.*" ), csPathname );

	// start trolling for files we are interested in
	CFileFind finder;
	BOOL bWorking = finder.FindFile( strWildcard );
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();

		// skip "." and ".." folder names
		if ( finder.IsDots() )
		{
			continue;
		}

		const CString csFolder =
			finder.GetFilePath().TrimRight( _T( "\\" ) );

		// if it's a directory, recursively search it
		if ( finder.IsDirectory() )
		{
			RecursePath( csFolder + _T( "\\" ) );
		}
		else // process the file if it is an extension we are interested in
		{
			vector<CString> tokens;
			const CString csPath = finder.GetFilePath();
			const CString csExt = CHelper::GetExtension( csPath ).MakeLower();
			const CString csFile = CHelper::GetFileName( csPath );
			const CString csFolder = CHelper::GetDirectory( csPath );
			
			if ( csExt == _T( ".docx" ) || csExt == _T( ".doc" ) )
			{
				// parse the sub-folders
				const CString csDelim( _T( "\\" ) );
				int nStart = 0;
				CString csToken;
				do
				{
					csToken = csFolder.Tokenize( csDelim, nStart );
					if ( csToken.IsEmpty() )
					{
						break;
					}

					tokens.push_back( csToken );

				}
				while ( true );

				// the last sub-folder
				const CString csSub = tokens.back();

				vector<CString>* pData = nullptr;
				if ( m_mapFilenames.Exists[ csFile ] )
				{
					pData = m_mapFilenames.find( csFile );
				}
				else // add the filename to the collection
				{	
					pData = new vector<CString>;
					m_mapFilenames.add( csFile, pData );
				}

				// add the sub-folder to the filename's collection
				pData->push_back( csSub );
			}
		}
	}

	finder.Close();

} // RecursePath

/////////////////////////////////////////////////////////////////////////////
// a console application that can crawl through the file
// and build a sorted list of filenames
int _tmain( int argc, TCHAR* argv[], TCHAR* envp[] )
{
	HMODULE hModule = ::GetModuleHandle( NULL );
	if ( hModule == NULL )
	{
		_tprintf( _T( "Fatal Error: GetModuleHandle failed\n" ) );
		return 1;
	}

	// initialize MFC and error on failure
	if ( !AfxWinInit( hModule, NULL, ::GetCommandLine(), 0 ) )
	{
		_tprintf( _T( "Fatal Error: MFC initialization failed\n " ) );
		return 2;
	}

	// do some common command line argument corrections
	vector<CString> arrArgs = CHelper::CorrectedCommandLine( argc, argv );
	size_t nArgs = arrArgs.size();

	// this stream can be redirected from the command line to allow the 
	// output you are interested in to be captured into another file
	// (Ex. > out_file.csv)
	CStdioFile fOut( stdout );

	// this stream is not redirected; it only shows up on the console and
	// will not affect the output file that is being redirected to
	CStdioFile fErr( stderr );

	CString csMessage;

	// display the number of arguments if not 1 to help the user 
	// understand what went wrong if there is an error in the
	// command line syntax
	if ( nArgs != 1 )
	{
		fErr.WriteString( _T( ".\n" ) );
		csMessage.Format( _T( "The number of parameters are %d\n.\n" ), nArgs - 1 );
		fErr.WriteString( csMessage );

		// display the arguments
		for ( int i = 1; i < nArgs; i++ )
		{
			csMessage.Format( _T( "Parameter %d is %s\n.\n" ), i, arrArgs[ i ] );
			fErr.WriteString( csMessage );
		}
	}

	// two or three arguments expected
	if ( !( nArgs == 2 || nArgs == 3 ))
	{
		fErr.WriteString( _T( ".\n" ) );
		fErr.WriteString
		(
			_T( "ScanForFiles, Copyright (c) 2022, " )
			_T( "by W. T. Block.\n" )
		);

		fErr.WriteString
		(
			_T( ".\n" )
			_T( "A Windows command line program to sort Word file(s) in\n" )
			_T( "  the given directory tree by their names and\n" )
			_T( "  output a CSV format with unique filenames \n" )
			_T( "  comma separated from sub-folder the file is\n" )
			_T( ". located in. Duplicate files will be listed for\n" )
			_T( ". each of their hosting sub-folders.\n" )
			_T( ".\n" )
		);

		fErr.WriteString
		(
			_T( ".\n" )
			_T( "Usage:\n" )
			_T( ".\n" )
			_T( ".  ScanForFiles pathname [true]\n" )
			_T( ".\n" )
			_T( "Where:\n" )
			_T( ".\n" )
		);

		fErr.WriteString
		(
			_T( ".  pathname is the root of the tree to be scanned\n" )
			_T( ".  true is optional to only show duplications\n" )
			_T( ".\n" )
			_T( ".  output \"unique_file_name\",\"sub_folder\"\n" )
			_T( ".\n" )
			_T( ".duplicate filenames will have multiple sub_folders\n" )

		);

		return 3;
	}

	// display the executable path
	//csMessage.Format( _T( "Executable pathname: %s\n" ), arrArgs[ 0 ] );
	//fErr.WriteString( _T( ".\n" ) );
	//fErr.WriteString( csMessage );
	//fErr.WriteString( _T( ".\n" ) );

	// retrieve the pathname
	CString csPath = arrArgs[ 1 ];
	m_bDuplicatesOnly = false;

	// optional argument to display duplicates only
	if ( nArgs == 3 )
	{
		if ( arrArgs[ 2 ] == _T( "true" ) )
		{
			m_bDuplicatesOnly = true;
		}
	}

	// trim off any wild card data
	const CString csFolder = CHelper::GetFolder( csPath );

	// test for current folder character (a period)
	bool bExists = csPath == _T( "." );

	// if it is a period, add a wild card of *.* to retrieve
	// all folders and files
	if ( bExists )
	{
		csPath = _T( ".\\*.*" );
		TCHAR pStr[ MAX_PATH ];
		::GetCurrentDirectory( MAX_PATH, pStr );
		m_csRoot = pStr;
	}
	else // if it is not a period, test to see if the folder exists
	{
		if ( ::PathFileExists( csFolder ) )
		{
			bExists = true;
			m_csRoot = csFolder;
		}
	}

	if ( !bExists )
	{
		csMessage.Format( _T( "Invalid pathname:\n\t%s\n" ), csPath );
		fErr.WriteString( _T( ".\n" ) );
		fErr.WriteString( csMessage );
		fErr.WriteString( _T( ".\n" ) );
		return 4;

	}
	else
	{
		csMessage.Format( _T( "Given pathname:\n\t%s\n" ), csPath );
		fErr.WriteString( _T( ".\n" ) );
		fErr.WriteString( csMessage );
	}


	// crawl through directory tree defined by the command line
	// parameter trolling for files
	RecursePath( csPath );

	// loop through the unique filenames 
	for ( auto& nodeKeys : m_mapFilenames.Items )
	{
		// loop through the sub-folders where the file was found
		// and write the pair onto the output separated by a comma
		vector<CString>* pData = nodeKeys.second;

		// if only outputting duplicates, continue when the count is one
		const size_t nCount = pData->size();
		if ( m_bDuplicatesOnly && nCount == 1 )
		{
			continue;
		}

		// generate a line of output
		for ( auto& nodeData : *pData )
		{
			csMessage.Format( _T( "\"%s\",\"%s\"\n" ), nodeKeys.first, nodeData );
			fOut.WriteString( csMessage );
		}
	}

	// all is good
	return 0;

} // _tmain

/////////////////////////////////////////////////////////////////////////////
