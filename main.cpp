#include <iostream>

#include "DBXV2/BcmFile.h"
#include "DBXV2/BacFile.h"
#include "debug.h"

static void wait_key()
{
	std::cin.ignore();
	do 
	{
	} while (std::cin.get() != '\n');
}

static bool load_file(BaseFile *file, const std::string &path)
{
	if (!file->LoadFromFile(path, false))
	{
		DPRINTF("Failed to load file \"%s\"\n", path.c_str());
		DPRINTF("Make sure to copy the files in the program directory or current working directory.\n");
		return false;
	}
	
	return true;
}

#define MIN_BAC_ENTRIES	477

//static const std::vector<uint32_t>  bac_entries_to_copy = { 453, 470, 475, 476 };
static const std::vector<uint32_t>  bac_entries_to_copy = { 475, 476 };

static bool qualifies_for_src(const BacFile &src, const std::string &path)
{
	if (src.GetNumEntries() < MIN_BAC_ENTRIES)
	{
		DPRINTF("\%s\" has less than %d entries\n", path.c_str(), MIN_BAC_ENTRIES);
		return false;
	}
	
	for (uint32_t idx : bac_entries_to_copy)
	{
		if (!src[idx].valid)
		{
			DPRINTF("\"%s\": entry %d is empty.\n", path.c_str(), idx);
			return false;
		}
	}
	
	return true;
}

static bool qualifies_for_dst(const BacFile &dst, const std::string &path)
{
	if (dst.GetNumEntries() < MIN_BAC_ENTRIES)
	{
		DPRINTF("\%s\" has less than %d entries\n", path.c_str(), MIN_BAC_ENTRIES);
		return false;
	}
	
	for (uint32_t idx : bac_entries_to_copy)
	{
		if (dst[idx].valid)
		{
			DPRINTF("\"%s\": entry %d is not empty.\n", path.c_str(), idx);
			return false;
		}
	}
	
	return true;
}

#define MIN_BCM_ENTRIES	5
#define BAC_ENTRY	476

static bool qualifies_for_src(const BcmFile &src, const std::string &path)
{
	if (src.GetNumEntries() < MIN_BCM_ENTRIES)
	{
		DPRINTF("\%s\" has less than %d entries\n", path.c_str(), MIN_BCM_ENTRIES);
		return false;
	}
	
	if (src[3].bac_entry_to_activate != BAC_ENTRY)
	{
		DPRINTF("\%s\": bac entry to activate in entry 3 is not %d.\n", path.c_str(), BAC_ENTRY);
		return false;
	}
	
	if (src[4].bac_entry_to_activate != BAC_ENTRY)
	{
		DPRINTF("\%s\": bac entry to activate in entry 4 is not %d.\n", path.c_str(), BAC_ENTRY);
		return false;
	}
	
	return true;
}

static bool qualifies_for_dst(const BcmFile &dst, const std::string &path)
{
	if (dst.GetNumEntries() < MIN_BCM_ENTRIES)
	{
		DPRINTF("\%s\" has less than %d entries\n", path.c_str(), MIN_BCM_ENTRIES);
		return false;
	}
	
	if (dst[3].bac_entry_to_activate == BAC_ENTRY)
	{
		DPRINTF("\%s\": bac entry to activate in entry 3 is already %d.\n", path.c_str(), BAC_ENTRY);
		return false;
	}
	
	if (dst[4].bac_entry_to_activate == BAC_ENTRY)
	{
		DPRINTF("\%s\": bac entry to activate in entry 4 is already %d.\n", path.c_str(), BAC_ENTRY);
		return false;
	}
	
	return true;
}

static void build_bac(BacFile &dst, const BacFile &src)
{
	for (uint32_t idx : bac_entries_to_copy)
	{
		dst[idx] = src[idx];
	}
}

static void build_bcm(BcmFile &dst, const BcmFile &src)
{
	const BcmEntry &entry3 = src[3];
    const BcmEntry &entry4 = src[4];    

    std::vector<BcmEntry> &entries = dst.GetEntries();

    entries.insert(entries.begin()+3, entry3);
    entries.insert(entries.begin()+4, entry4);

    for (size_t i = 5; i < entries.size(); i++)
    {
        BcmEntry &entry = entries[i];

        if (entry.sibling != 0)
        {
            entry.sibling += (2*sizeof(BCMEntry));
        }

        if (entry.child != 0)
        {
             entry.child += (2*sizeof(BCMEntry));
        }

        if (entry.parent != 0)
        {
            entry.parent += (2*sizeof(BCMEntry));
        }

        if (entry.root != 0)
        {
            entry.root += (2*sizeof(BCMEntry));
        }
    }
}

int main(int argc, char *argv[])
{
	std::string src, dst;
	std::string src_path, dst_path;
	
	BacFile src_bac, dst_bac;
	BcmFile src_bcm, dst_bcm;
	
	UPRINTF("Enter the code of the char WITHOUT dual skill support: ");
	std::cin >> dst;
	
	UPRINTF("Enter the code of the char that HAS dual skill support: ");
	std::cin >> src;
	
	src_path = src + "_PLAYER";
	dst_path = dst + "_PLAYER";
	
	if (!load_file(&dst_bac, dst_path + ".bac"))
		goto leave;
	
	if (!load_file(&src_bac, src_path + ".bac"))
		goto leave;	
	
	if (!load_file(&dst_bcm, dst_path + ".bcm"))
		goto leave;
	
	if (!load_file(&src_bcm, src_path + ".bcm"))
		goto leave;
	
	if (!qualifies_for_dst(dst_bac, dst_path + ".bac"))
	{
		DPRINTF("\"%s\" doesn't qualify for bac without dual skill.\n", dst_path.c_str());
		goto leave;
	}
	
	if (!qualifies_for_src(src_bac, src_path + ".bac"))
	{
		DPRINTF("\"%s\" doesn't qualify for bac with dual skill.\n", src_path.c_str());
		goto leave;
	}
	
	if (!qualifies_for_dst(dst_bcm, dst_path + ".bcm"))
	{
		DPRINTF("\"%s\" doesn't qualify for bcm without dual skill.\n", dst_path.c_str());
		goto leave;
	}
	
	if (!qualifies_for_src(src_bcm, src_path + ".bcm"))
	{
		DPRINTF("\"%s\" doesn't qualify for bcm with dual skill.\n", src_path.c_str());
		goto leave;
	}
	
	build_bac(dst_bac, src_bac);
	build_bcm(dst_bcm, src_bcm);
	
	if (!dst_bac.SaveToFile(dst_path + ".bac"))
	{
		DPRINTF("Failed to save bac.\n");
		goto leave;
	}
	
	if (!dst_bcm.SaveToFile(dst_path + ".bcm"))
	{
		DPRINTF("Failed to save bcm.\n");
		goto leave;
	}
	
	UPRINTF("\nOperation completed with success\n");
	UPRINTF("Remember that the char must have a bdm assigned too in the cms (one of a char with dual skill support) or the kick will always miss!\n");
	
leave:
	
	UPRINTF("\nPress enter to exit.");
	wait_key();
	
	return 0;
}
