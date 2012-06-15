
#ifndef __AKNUTILS_H__
#define __AKNUTILS_H__

//TODO: temporary fix this
class AknLayoutUtils
{
public:
   enum TAknCbaLocation
   {
      EAknCbaLocationBottom, //landscape and portrait
      EAknCbaLocationRight,  //only landscape
      EAknCbaLocationLeft    //only landscape
   };

   static TAknCbaLocation CbaLocation();

   //SL: added
   static void SetCbaLocation(TAknCbaLocation aLocation);
};


#endif