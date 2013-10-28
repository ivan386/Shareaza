//
// GenreMap.cs
//
// Copyright (c) Shareaza Development Team, 2008.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#region Using directives
using System;
using System.Collections;
using System.Text;

#endregion

namespace Schemas
{
	internal static class GenreMap
	{
		static Hashtable _map = new Hashtable();
		static GenreMap()
		{
			_map.Add(genreType.adv_animal, ShareazaBook.RazaGenreType.OutdoorsNature);
			_map.Add(genreType.adv_geo, ShareazaBook.RazaGenreType.Travel);
			_map.Add(genreType.adv_history, ShareazaBook.RazaGenreType.History);
			_map.Add(genreType.adv_indian, ShareazaBook.RazaGenreType.Nonfiction);
			_map.Add(genreType.adv_maritime, ShareazaBook.RazaGenreType.Travel);
			_map.Add(genreType.adv_western, ShareazaBook.RazaGenreType.Nonfiction);
			_map.Add(genreType.adventure, ShareazaBook.RazaGenreType.Nonfiction);
			_map.Add(genreType.antique, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.antique_ant, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.antique_east, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.antique_european, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.antique_myths, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.antique_russian, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.child_adv, ShareazaBook.RazaGenreType.ChildrensBooks);
			_map.Add(genreType.child_det, ShareazaBook.RazaGenreType.ChildrensBooks);
			_map.Add(genreType.child_education, ShareazaBook.RazaGenreType.ChildrensBooks);
			_map.Add(genreType.child_prose, ShareazaBook.RazaGenreType.ChildrensBooks);
			_map.Add(genreType.child_sf, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.child_tale, ShareazaBook.RazaGenreType.ChildrensBooks);
			_map.Add(genreType.child_verse, ShareazaBook.RazaGenreType.ChildrensBooks);
			_map.Add(genreType.children, ShareazaBook.RazaGenreType.ChildrensBooks);
			_map.Add(genreType.comp_db, ShareazaBook.RazaGenreType.ComputersInternet);
			_map.Add(genreType.comp_hard, ShareazaBook.RazaGenreType.ComputersInternet);
			_map.Add(genreType.comp_osnet, ShareazaBook.RazaGenreType.ComputersInternet);
			_map.Add(genreType.comp_programming, ShareazaBook.RazaGenreType.ComputersInternet);
			_map.Add(genreType.comp_soft, ShareazaBook.RazaGenreType.ComputersInternet);
			_map.Add(genreType.comp_www, ShareazaBook.RazaGenreType.ComputersInternet);
			_map.Add(genreType.computers, ShareazaBook.RazaGenreType.ComputersInternet);
			_map.Add(genreType.design, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.det_action, ShareazaBook.RazaGenreType.MysteryThrillers);
			_map.Add(genreType.det_classic, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.det_crime, ShareazaBook.RazaGenreType.MysteryThrillers);
			_map.Add(genreType.det_espionage, ShareazaBook.RazaGenreType.Nonfiction);
			_map.Add(genreType.det_hard, ShareazaBook.RazaGenreType.MysteryThrillers);
			_map.Add(genreType.det_history, ShareazaBook.RazaGenreType.History);
			_map.Add(genreType.det_irony, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.det_maniac, ShareazaBook.RazaGenreType.MysteryThrillers);
			_map.Add(genreType.det_police, ShareazaBook.RazaGenreType.MysteryThrillers);
			_map.Add(genreType.det_political, ShareazaBook.RazaGenreType.MysteryThrillers);
			_map.Add(genreType.detective, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.dramaturgy, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.home, ShareazaBook.RazaGenreType.HomeGarden);
			_map.Add(genreType.home_cooking, ShareazaBook.RazaGenreType.CookingFoodWine);
			_map.Add(genreType.home_crafts, ShareazaBook.RazaGenreType.HomeGarden);
			_map.Add(genreType.home_diy, ShareazaBook.RazaGenreType.HomeGarden);
			_map.Add(genreType.home_entertain, ShareazaBook.RazaGenreType.Entertainment);
			_map.Add(genreType.home_garden, ShareazaBook.RazaGenreType.HomeGarden);
			_map.Add(genreType.home_health, ShareazaBook.RazaGenreType.HealthFitness);
			_map.Add(genreType.home_pets, ShareazaBook.RazaGenreType.HomeGarden);
			_map.Add(genreType.home_sex, ShareazaBook.RazaGenreType.HealthFitness);
			_map.Add(genreType.home_sport, ShareazaBook.RazaGenreType.Sports);
			_map.Add(genreType.humor, ShareazaBook.RazaGenreType.Comics);
			_map.Add(genreType.humor_anecdote, ShareazaBook.RazaGenreType.Comics);
			_map.Add(genreType.humor_prose, ShareazaBook.RazaGenreType.Comics);
			_map.Add(genreType.humor_verse, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.love_contemporary, ShareazaBook.RazaGenreType.Romance);
			_map.Add(genreType.love_detective, ShareazaBook.RazaGenreType.MysteryThrillers);
			_map.Add(genreType.love_erotica, ShareazaBook.RazaGenreType.Romance);
			_map.Add(genreType.love_history, ShareazaBook.RazaGenreType.History);
			_map.Add(genreType.love_short, ShareazaBook.RazaGenreType.Romance);
			_map.Add(genreType.nonf_biography, ShareazaBook.RazaGenreType.BiographiesMemoirs);
			_map.Add(genreType.nonf_criticism, ShareazaBook.RazaGenreType.Nonfiction);
			_map.Add(genreType.nonf_publicism, ShareazaBook.RazaGenreType.Nonfiction);
			_map.Add(genreType.nonfiction, ShareazaBook.RazaGenreType.Nonfiction);
			_map.Add(genreType.poetry, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.prose_classic, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.prose_contemporary, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.prose_counter, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.prose_history, ShareazaBook.RazaGenreType.History);
			_map.Add(genreType.prose_rus_classic, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.prose_su_classics, ShareazaBook.RazaGenreType.LiteratureFiction);
			_map.Add(genreType.ref_dict, ShareazaBook.RazaGenreType.Reference);
			_map.Add(genreType.ref_encyc, ShareazaBook.RazaGenreType.Reference);
			_map.Add(genreType.ref_guide, ShareazaBook.RazaGenreType.Reference);
			_map.Add(genreType.ref_ref, ShareazaBook.RazaGenreType.Reference);
			_map.Add(genreType.reference, ShareazaBook.RazaGenreType.Reference);
			_map.Add(genreType.religion, ShareazaBook.RazaGenreType.ReligionSpirituality);
			_map.Add(genreType.religion_esoterics, ShareazaBook.RazaGenreType.ReligionSpirituality);
			_map.Add(genreType.religion_rel, ShareazaBook.RazaGenreType.ReligionSpirituality);
			_map.Add(genreType.religion_self, ShareazaBook.RazaGenreType.MindBody);			
			_map.Add(genreType.sci_biology, ShareazaBook.RazaGenreType.Science);
			_map.Add(genreType.sci_business, ShareazaBook.RazaGenreType.BusinessInvesting);
			_map.Add(genreType.sci_chem, ShareazaBook.RazaGenreType.Science);
			_map.Add(genreType.sci_culture, ShareazaBook.RazaGenreType.Science);
			_map.Add(genreType.sci_history, ShareazaBook.RazaGenreType.History);
			_map.Add(genreType.sci_juris, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_linguistic, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_math, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_medicine, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_philosophy, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_phys, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_politics, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_psychology, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.sci_religion, ShareazaBook.RazaGenreType.ReligionSpirituality);
			_map.Add(genreType.sci_tech, ShareazaBook.RazaGenreType.ProfessionalTechnical);
			_map.Add(genreType.science, ShareazaBook.RazaGenreType.Science);
			_map.Add(genreType.sf, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_action, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_cyberpunk, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_detective, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_epic, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_fantasy, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_heroic, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_history, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_horror, ShareazaBook.RazaGenreType.Horror);
			_map.Add(genreType.sf_humor, ShareazaBook.RazaGenreType.Comics);
			_map.Add(genreType.sf_social, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.sf_space, ShareazaBook.RazaGenreType.ScienceFictionFantasy);
			_map.Add(genreType.thriller, ShareazaBook.RazaGenreType.MysteryThrillers);
	}
		
		public static ShareazaBook.RazaGenreType GetRazaGenre(genreType fbGenre) {
			return (ShareazaBook.RazaGenreType)_map[fbGenre];
		}
	}
}
