#include <stdio.h>
#include <iostream.h>


#include "../makedoc9.h"

void main () 
{
	tBuf fText;
	char*text="asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf";

	fText.setText((const byte*)text);
	cout<<"Decompressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;
	
	fText.Compress();
	cout<<"  Compressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;
	fText.Decompress();
	cout<<"Decompressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;

	fText.Compress();
	cout<<"  Compressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;
	fText.Decompress();
	cout<<"Decompressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;

	fText.Compress();
	cout<<"  Compressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;
	fText.Decompress();
	cout<<"Decompressed text: "<<fText.text()<<"  ("<<fText.Len()<<")  --  Compressed: "<<fText.compressed()<<endl;


}
