using System;
using System.Collections;
using System.Text;

namespace Schemas
{
	internal static class GenreMap
	{
		static Hashtable _map = new Hashtable();
		static GenreMap()
		{
			// ToDO: map all genres
			_map.Add(genreType.adv_animal, ShareazaBook.RazaGenreType.OutdoorsNature);
		}
		
		public static ShareazaBook.RazaGenreType GetRazaGenre(genreType fbGenre) {
			return (ShareazaBook.RazaGenreType)_map[fbGenre];
		}
	}
}
