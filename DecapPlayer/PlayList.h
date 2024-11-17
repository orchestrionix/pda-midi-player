// PlayList.h: interface for the CPlayList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYLIST_H__6AACB122_6842_40DF_894F_9D7A0CBEF7C9__INCLUDED_)
#define AFX_PLAYLIST_H__6AACB122_6842_40DF_894F_9D7A0CBEF7C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPlayList  
{
public:
	enum {
		MAX_LIST = 3999
	};

	void Sort(bool isAscend);
	LPCTSTR GetDisplayName(int id);
	void Add(LPCTSTR fn);
	LPCTSTR GetListFile();
	void SetShuffle(bool shuffled);
	bool Open(HWND hWnd);
	bool Open(LPCTSTR fn);
	bool Open(HWND hwnd, int id);
	bool Save(HWND hWnd);
	bool SaveAs(HWND hWnd);
	void NewList();
	void Insert(int id, LPTSTR fn);
	void Swap(int id1, int id2);
	void Delete(int id);
	bool ShowDlgAdd(HWND hwnd);
	void RemoveAll();

	LPCTSTR GetFileName(int id){
		ASSERT(id < m_nlist);
		return m_list[id];
	}

	LPCTSTR GetItemName(int id){
		ASSERT(id < m_nlist);
		return m_list[id];
	}

	LPCTSTR GetShuffledFileName(int id){
		ASSERT(id < m_nlist);
		return m_list[m_order[id]];
	}

	LPCTSTR GetShuffledItemName(int id){
		ASSERT(id < m_nlist);
		return m_list[m_order[id]];
	}

	int GetItemCount(){
		return m_nlist;
	}

	bool GetShuffle(){
		return m_shuffled;
	}

	int GetShuffledOrder(int fakeid)
	{
		ASSERT(fakeid < m_nlist);
		return m_order[fakeid];
	}

	CPlayList();
	virtual ~CPlayList();

protected:
	bool Save(LPCTSTR fn);
	LPTSTR m_list[MAX_LIST];
	int m_nlist;
	TCHAR m_listfn[MAX_PATH];
	unsigned short m_order[MAX_LIST];
	bool m_shuffled;
};

extern CPlayList g_playlist;
#endif // !defined(AFX_PLAYLIST_H__6AACB122_6842_40DF_894F_9D7A0CBEF7C9__INCLUDED_)
