# Scrapes superfamicom.org's game list to create a databae of cartridges
# Use like 
# python scrape_db.py > snes_db.json 

import requests
import re

mapper_pattern = re.compile(r"ROM Bank<\/strong>\n\s*\s*<code>(.*)</code>", re.IGNORECASE)
sha_pattern = re.compile(r"SHA-1<\/strong>\n\s*\s*<code>(.*)</code>", re.IGNORECASE)
rom_size_pattern = re.compile(r"ROM Size<\/strong>\n\s*\s*<code>(.*) Mb</code>", re.IGNORECASE)
ram_size_pattern = re.compile(r"SRAM Size<\/strong>\n\s*\s*<code>(.*) Kb</code>", re.IGNORECASE)
rom_type_pattern = re.compile(r"ROM type<\/strong>\n\s*\s*<code>(.*)</code>", re.IGNORECASE)
region_pattern = re.compile(r"Country<\/strong>\n\s*\s*<code>(.*)</code>", re.IGNORECASE)
scrape_titles_pattern = re.compile(r"<a href=\"\/info(.*)\" title=.+?(?=>)")

print ("{") # Start of JSON file
for i in range (1, 25): # Scan all 24 pages of the game list
    URL = "https://superfamicom.org/game-list/" + str(i)
    page = requests.get(URL)

    for title in scrape_titles_pattern.findall(page.text): # Get each game URL in the page
        title_url = "https://superfamicom.org/info" + title
        title_page = requests.get(title_url).text

        mappers = mapper_pattern.findall(title_page)
        shas = sha_pattern.findall(title_page)
        rom_sizes = rom_size_pattern.findall(title_page)
        ram_sizes = ram_size_pattern.findall(title_page)
        rom_types = rom_type_pattern.findall(title_page)
        regions = region_pattern.findall(title_page)

        for idx, sha in enumerate(shas): # Every game page can have multiple carts, due to regional differences
            # Skip carts that have unlisted ROM information, by checking if more SHA-1 checksums are present than ROM info
            if idx >= len(mappers):
                break

            print("\"", sha, "\": {", sep="")
            print ("\t\"Mapper\": \"", mappers[idx], "\",", sep="")
            print ("\t\"ROMType\": \"", rom_types[idx], "\",", sep="")
            print ("\t\"ROMSize\": ", rom_sizes[idx], ",", sep="") # In Megabits

            if idx < len(ram_sizes): # Handle carts that don't specify SRAM size, set the size to 0
                print ("\t\"RAMSize\": ", ram_sizes[idx], ",", sep="")
            else:
                print ("\t\"RAMSize\": 0,")

            print ("\t\"Region\": \"", regions[idx], "\"", sep="")
            print ("},")
print ("}")