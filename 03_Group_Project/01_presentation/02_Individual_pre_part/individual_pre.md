## Dressing in Order

Our topic is text2human. First of all, let me introduce what it is.

### Introduction

It is desirable to intuitively control the synthesized human images for layman users. For example, they may want to generate a person wearing a floral T-shirt and jeans without expert software knowledge. 
This is why text2human was created, and human image generation with explicit textual controls makes it possible for users to create 2D avatars more easily.

So how it works? It has two stages. First, it generates a human parsing mask with diverse clothes shapes based on the given human pose and user-specified texts describing the clothes shapes. Second, it enriches the human parsing mask with diverse textures of clothes based on texts describing the clothes textures.

Different from the main topic paper, this paper not only focuses on encoding pose, skin and garments, but also pays attention to the order when putting on garments. And it is designed for multiple fashion tasks including virtual try-on. 

let's assume we have a source person and two garments to try on. Dressing in order can let the user to decide what order they want to put on the shirt or the pants. As a result, it will generate either tucking-in result or not tucking-in result. Besides that we can also keep layering additional garments on the existing outfits. In this way it can achieve both multiple layering and multiple layouts.

The key for dressing in order to work on multiple tasks is a recurrent pipeline. The generation starts with a target pose and the source person. The first thing it does is to extract the skin region from the source person, and render the source person in the target pose. Next, for every garment that we are interested, it will separately encode its shape and texture.

And it uses a generator called "G_gar" to put it back on the person. the "G_gar" will be recurrently running for every new garment, so it can sequentially layer in garments one by one. Technically we can keep running this step as many times as we want. Because this is a recurrent mechanism with such a pipeline. 



We can use every component to achieve different applications. For example we can achieve pose transfer by using this target pose input. Also, we can use the dressing order. 

In addition, it can preserve more details in terms of shape and texture, more results about the try-on, including try-on with different layout,  layering inside and outside and keep layering outside, more results about editing, including texture transfer, reshaping, content removal and content insertion.
